/***************************************************************************
 graphics_png.c Copyright (C) 1999 Christoph Reichenbach, TU Darmstadt


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

    Christoph Reichenbach (CJR) [creichen@rbg.informatik.tu-darmstadt.de]

***************************************************************************/
/* Provides facilities for writing pictures and seperate views to .png files */


#ifndef _SCI_GRAPHICS_PNG_H_
#define _SCI_GRAPHICS_PNG_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_LIBPNG

#include <resource.h>
#include <graphics.h>
#include <png.h>

int
write_pic_png(char *filename, guint8 *pic);
/* Stores a picture map in a png file
** --DEPRECATED--
** Parameters: (filename) The name of the file to create
**             (pic) A pointer to the picture map to store
** Returns   : (int) 0 on success, != 0 otherwise
** Please note that pic must not point to a picture_t, it must point to a
** map.
** E.g. to store the foreground picture, you could use
** write_pic_png("filename.png", picture->maps[0]).
*/


int
png_save_buffer(picture_t pic, char *name,
                int xoffset, int yoffset, int width, int height,
                byte *data, int force_8bpp_special);
/* Stores any buffer in a png file
** Parameters: (picture_t) pic: The picture_t containing the parameters of the buffer
**             (char *) name: File name to write to
**             (int x int) xoffset, yoffset: relative screen position of the buffer
**             (int x int) width, height: Buffer size
**             (byte *) data: The actual data to write
**             (int) force_8bpp_special: See below
** Returns   : (int) 0 on success, 1 otherwise
** Each graphics buffer is stored in the way that the corresponding picture_t
** dictates (all buffers are either identical to, or parts of a past representation
** of the picture_t map 0, after all).
** force_8bpp_special overrides this and forces 8bpp palette mode for writing the
** buffer.
** This should only be used for maps 1 through 3, which are always in this format.
*/


byte *
png_load_buffer(picture_t pic, char *name,
                int *xoffset, int *yoffset, int *width, int *height,
                int *size, int force_8bpp_special);
/* Loads a buffer from a png file
** Parameters: (picture_t) pic: The picture_t containing the target parameters
**             (char *) name: The file name of the file to read from
**             (int* x int*) xoffset, yoffset: Offset storage pointers
**             (int* x int*) width, height: Size storage pointers
**             (int*) size: Memory size storage pointer
**             (int) force_8bpp_special: Import file as 8bpp with palette
** Returns   : (byte *): The buffer on success, NULL otherwise
** xoffset, yoffset, width, height, and size should each point to ints the picture
** parameters are stored in. xoffset and yoffset may be set to NULL, though, in
** order to be omitted.
*/


int
png_save_pic(picture_t pic);
/* Stores a picture_t in the current directory
** Parameters: (picture_t) pic: The picture_t to store
** Returns   : (int) 0 on success, 1 otherwise
*/

int
png_load_pic(picture_t pic);
/* Loads a picture_t from the cwd
** Parameters: (picture_t) pic: The structure to write to
** Returns   : (int) 0 on success, 1 otherwise
** On error, the original pic status is retained.
*/


#endif /* HAVE_LIBPNG */
#endif /* !_SCI_GRAPHICS_PNG_H_ */
