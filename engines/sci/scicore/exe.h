/***************************************************************************
 exe.h Copyright (C) 2005 Walter van Niftrik


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

#ifndef _SCI_EXE_H_
#define _SCI_EXE_H_

typedef struct _exe_file exe_file_t;

exe_file_t *
exe_open(const char *filename);
/* Opens an executable file
** Parameters: (const char *) filename: Filename of executable to open
** Returns   : (exe_file_t *) File handle, or NULL on error
** This function will try to find a decompressor that can handle this type
** of executable
*/

int
exe_read(exe_file_t *file, void *buf, int count);
/* Reads from an executable file
** Parameters: (exe_file_t *) file: File handle
**             (void *) buf: Buffer to store decompressed data
**             (int) count: Size of decompressed data requested, in bytes
** Returns   : (int) Number of bytes of decompressed data that was stored in
**                   buf. If this value is less than count an error has
**                   occured, or end-of-file was reached.
*/

void
exe_close(exe_file_t *handle);
/* Closes an executable file
** Parameters: (exe_file_t *) file: File handle
** Returns   : (void)
*/

#endif /* !_SCI_EXE_H_ */
