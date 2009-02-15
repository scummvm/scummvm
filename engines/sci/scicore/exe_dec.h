/***************************************************************************
 exe_dec.h Copyright (C) 2005 Walter van Niftrik


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

#ifndef _SCI_EXE_DEC_H_
#define _SCI_EXE_DEC_H_

typedef struct _exe_handle exe_handle_t;

typedef struct _exe_decompressor {
	const char *name; /* Decompressor name. Unique identifier, should consist
	            ** of lower-case (where applicable) alphanumerics
	            */

	exe_handle_t * (*open)(const char *filename);
	/* Opens an executable file
	** Parameters: (const char *) filename: Filename of executable to open.
	** Returns   : (exe_handle_t *) Decompressor file handle, or NULL on
	**                              error.
	** This function will verify that the file can be handled by the
	** decompressor. If this is not the case the function will fail.
	*/

	int (*read)(exe_handle_t *handle, void *buf, int count);
	/* Reads from executable file
	** Parameters: (exe_handle_t *) handle: Decompressor file handle.
	**             (void *) buf: Buffer to store decompressed data.
	**             (int) count: Size of decompressed data requested, in
	**                          bytes.
	** Returns   : (int) Number of bytes of decompressed data that was
	**                   stored in buf. If this value is less than count
	**                   an error has occured, or end-of-file was
	**                   reached.
	*/

	void (*close)(exe_handle_t *handle);
	/* Closes a decompressor file handle.
	** Parameters: (exe_handle_t *) handle: Decompressor file handle.
	** Returns   : (void)
	*/
} exe_decompressor_t;

#endif /* !_SCI_EXE_DEC_H_ */
