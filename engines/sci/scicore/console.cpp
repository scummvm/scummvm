/***************************************************************************
 console.c Copyright (C) 1999..2002 Christoph Reichenbach, TU Darmstadt


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
/* First part of the console implmentation: VM independent stuff */
/* Remember, it doesn't have to be fast. */

#include "sci/include/sci_memory.h"
#include "sci/include/engine.h"
#ifdef SCI_CONSOLE

int con_passthrough = 0;
FILE *con_file = NULL;

static void(*_con_string_callback)(char*) = NULL;
static void (*_con_pixmap_callback)(gfx_pixmap_t *) = NULL;


/****************************************/
/* sciprintf                            */
/****************************************/


int
sciprintf(const char *fmt, ...) {
	va_list argp;
	size_t bufsize = 256;
	unsigned int i;
	char *buf 	= (char *) sci_malloc(bufsize);

	if (NULL == fmt) {
		fprintf(stderr, "console.c: sciprintf(): NULL passed for parameter fmt\n");
		return -1;
	}

	if (NULL == buf) {
		fprintf(stderr, "console.c: sciprintf(): malloc failed for buf\n");
		return -1;
	}

	va_start(argp, fmt);
	while ((i = vsnprintf(buf, bufsize - 1, fmt, argp)) == -1
	        || (i >= bufsize - 2)) {
		/* while we're out of space... */
		va_end(argp);
		va_start(argp, fmt);	/* reset argp */

		free(buf);
		buf = (char *) sci_malloc(bufsize <<= 1);
	}
	va_end(argp);

	if (con_passthrough)
		printf("%s", buf);
	if (con_file)
		fprintf(con_file, "%s", buf);


	if (_con_string_callback)
		_con_string_callback(buf);
	else
		free(buf);

	return 1;
}

void
con_set_string_callback(void(*callback)(char *)) {
	_con_string_callback = callback;
}

void
con_set_pixmap_callback(void(*callback)(gfx_pixmap_t *)) {
	_con_pixmap_callback = callback;
}

int
con_can_handle_pixmaps(void) {
	return _con_pixmap_callback != NULL;
}

int
con_insert_pixmap(gfx_pixmap_t *pixmap) {
	if (_con_pixmap_callback)
		_con_pixmap_callback(pixmap);
	else
		return 1;
	return 0;
}


void
open_console_file(char *filename) {
	if (con_file != NULL)
		fclose(con_file);

	if (NULL == filename) {
		fprintf(stderr, "console.c: open_console_file(): NULL passed for parameter filename\r\n");
	}
#ifdef WIN32
	con_file = fopen(filename, "wt");
#else
	con_file = fopen(filename, "w");
#endif

	if (NULL == con_file)
		fprintf(stderr, "console.c: open_console_file(): Could not open output file %s\n", filename);

}

void
close_console_file(void) {
	if (con_file != NULL) {
		fclose(con_file);
		con_file = NULL;
	}
}


#endif /* SCI_CONSOLE */
