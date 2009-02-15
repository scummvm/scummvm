/*
 * Copyright 2001 Computing Research Labs, New Mexico State University
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COMPUTING RESEARCH LAB OR NEW MEXICO STATE UNIVERSITY BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
 * OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef lint
#ifdef __GNUC__
static char rcsid[] __attribute__((unused)) = "$Id: bdfgname.c 1284 2004-04-02 07:42:44Z jameson $";
#else
static char rcsid[] = "$Id: bdfgname.c 1284 2004-04-02 07:42:44Z jameson $";
#endif
#endif

#include "bdfP.h"

typedef struct {
	long code;
	long start;
	long end;
	long pad;
} _bdf_adobe_name_t;

static _bdf_adobe_name_t *adobe_names;
static unsigned long adobe_names_size;
static unsigned long adobe_names_used;

/*
 * Provide a maximum length for glyph names just to make things clearer.
 */
#define MAX_GLYPH_NAME_LEN 127

static int
#ifdef __STDC__
getline(FILE *in, char *buf, int limit)
#else
getline(in, buf, limit)
FILE *in;
char *buf;
int limit;
#endif
{
	int c, i;

	c = EOF;

	for (i = 0; i < limit - 1; i++) {
		if ((c = getc(in)) == EOF || (c == '\n' || c == '\r'))
			break;
		buf[i] = c;
	}
	buf[i] = 0;

	/*
	 * Discard the rest of the line which did not fit into the buffer.
	 */
	while (c != EOF && c != '\n' && c != '\r')
		c = getc(in);

	if (c == '\r') {
		/*
		 * Check for a trailing newline.
		 */
		c = getc(in);
		if (c != '\n')
			ungetc(c, in);
	}

	return i;
}

static long
#ifdef __STDC__
_bdf_find_name(long code, char *name, FILE *in)
#else
_bdf_find_name(code, name, in)
long code;
char *name;
FILE *in;
#endif
{
	long c, i, pos;
	char *sp, buf[256];

	while (!feof(in)) {
		pos = ftell(in);
		(void) getline(in, buf, 256);
		while (!feof(in) && (buf[0] == 0 || buf[0] == '#')) {
			buf[0] = 0;
			pos = ftell(in);
			(void) getline(in, buf, 256);
		}

		if (buf[0] == 0)
			return -1;

		c = _bdf_atol(buf, 0, 16);

		if (c > code) {
			/*
			 * Restore the last position read in case the code is not in the
			 * file and the current code is greater than the expected code.
			 */
			fseek(in, pos, 0L);
			return -1;
		}

		if (c == code) {
			for (sp = buf; *sp != ';'; sp++) ;
			sp++;
			for (i = 0; *sp != ';' && i < MAX_GLYPH_NAME_LEN; sp++, i++)
				name[i] = *sp;
			name[i] = 0;
			return i;
		}
	}
	return -1;
}

static int
#ifdef __STDC__
by_encoding(const void *a, const void *b)
#else
by_encoding(a, b)
char *a, *b;
#endif
{
	_bdf_adobe_name_t *c1, *c2;

	c1 = (_bdf_adobe_name_t *) a;
	c2 = (_bdf_adobe_name_t *) b;
	if (c1->code < c2->code)
		return -1;
	else if (c1->code > c2->code)
		return 1;
	return 0;
}

static void
#ifdef __STDC__
_bdf_load_adobe_names(FILE *in)
#else
_bdf_load_adobe_names(in)
FILE *in;
#endif
{
	long c, pos;
	char *sp, buf[256];

	/*
	 * Go back to the beginning of the file to look for the code because the
	 * codes are not in order in the current Adobe Glyph Name list file.
	 */
	fseek(in, 0, 0);

	while (!feof(in)) {
		pos = ftell(in);
		(void) getline(in, buf, 256);
		while (!feof(in) && (buf[0] == 0 || buf[0] == '#')) {
			buf[0] = 0;
			pos = ftell(in);
			(void) getline(in, buf, 256);
		}

		c = _bdf_atol(buf, 0, 16);

		/*
		 * Ignore the Adobe-specific names in the Private Use Area.
		 */
		if (c >= 0xe000 && c <= 0xf8ff)
			continue;

		if (adobe_names_used == adobe_names_size) {
			if (adobe_names_size == 0)
				adobe_names = (_bdf_adobe_name_t *)
				              malloc(sizeof(_bdf_adobe_name_t) << 9);
			else
				adobe_names = (_bdf_adobe_name_t *)
				              realloc((char *) adobe_names,
				                      sizeof(_bdf_adobe_name_t) *
				                      (adobe_names_size + 512));
			(void) memset((char *)(adobe_names + adobe_names_size), 0,
			              sizeof(_bdf_adobe_name_t) << 9);
			adobe_names_size += 512;
		}

		adobe_names[adobe_names_used].code = c;
		for (sp = buf; *sp != ';'; sp++) ;
		sp++;
		adobe_names[adobe_names_used].start = pos + (sp - buf);
		for (; *sp != ';'; sp++) ;
		adobe_names[adobe_names_used].end = pos + (sp - buf);
		adobe_names_used++;
	}

	/*
	 * Sort the results by code.
	 */
	qsort((char *) adobe_names, adobe_names_used, sizeof(_bdf_adobe_name_t),
	      by_encoding);
}

static long
#ifdef __STDC__
_bdf_find_adobe_name(long code, char *name, FILE *in)
#else
_bdf_find_adobe_name(code, name, in)
long code;
char *name;
FILE *in;
#endif
{
	long len;
	int l, r, m;

	if (code < 0x20 || (code >= 0x7f && code <= 0x9f) ||
	        code == 0xfffe || code == 0xffff) {
		sprintf(name, "char%lu", code);
		return (long) strlen(name);
	}

	if (code >= 0xe000 && code <= 0xf8ff) {
		sprintf(name, "uni%04lX", code & 0xffff);
		return (long) strlen(name);
	}

	if (adobe_names_size == 0)
		_bdf_load_adobe_names(in);

	l = 0;
	r = adobe_names_used - 1;
	while (l <= r) {
		m = (l + r) >> 1;
		if (adobe_names[m].code < code)
			l = m + 1;
		else if (adobe_names[m].code > code)
			r = m - 1;
		else {
			fseek(in, adobe_names[m].start, 0);
			len = adobe_names[m].end - adobe_names[m].start;
			if (len > MAX_GLYPH_NAME_LEN)
				len = MAX_GLYPH_NAME_LEN;
			len = (long) fread(name, sizeof(char), len, in);
			name[len] = 0;
			return len;
		}
	}

	sprintf(name, "uni%04lX", code & 0xffff);
	return (long) strlen(name);
}

static int
#ifdef __STDC__
_bdf_set_glyph_names(FILE *in, bdf_font_t *font, bdf_callback_t callback,
                     int adobe)
#else
_bdf_set_glyph_names(in, font, callback, adobe)
FILE *in;
bdf_font_t *font;
bdf_callback_t callback;
int adobe;
#endif
{
	int changed;
	long i, size, len;
	bdf_glyph_t *gp;
	bdf_callback_struct_t cb;
	char name[MAX_GLYPH_NAME_LEN + 1];

	if (callback != 0) {
		cb.reason = BDF_GLYPH_NAME_START;
		cb.current = 0;
		cb.total = font->glyphs_used;
		(*callback)(&cb, 0);
	}
	for (changed = 0, i = 0, gp = font->glyphs; i < font->glyphs_used;
	        i++, gp++) {
		size = (adobe) ?
		       _bdf_find_adobe_name(gp->encoding, name, in) :
		       _bdf_find_name(gp->encoding, name, in);
		if (size < 0)
			continue;

		len = (gp->name) ? strlen(gp->name) : 0;
		if (len == 0) {
			gp->name = (char *) _bdf_strdup((unsigned char *) name, size + 1);
			changed = 1;
		} else if (size != len || strcmp(gp->name, name) != 0) {
			/*
			 * Simply resize existing storage so lots of memory allocations
			 * are not needed.
			 */
			if (size > len)
				gp->name = (char *) realloc(gp->name, size + 1);
			(void) strcpy(gp->name, name);
			changed = 1;
		}

		if (callback != 0) {
			cb.reason = BDF_GLYPH_NAME;
			cb.current = i;
			(*callback)(&cb, 0);
		}
	}

	if (callback != 0) {
		cb.reason = BDF_GLYPH_NAME;
		cb.current = cb.total;
		(*callback)(&cb, 0);
	}

	return changed;
}

int
#ifdef __STDC__
bdf_set_unicode_glyph_names(FILE *in, bdf_font_t *font,
                            bdf_callback_t callback)
#else
bdf_set_unicode_glyph_names(in, font, callback)
FILE *in;
bdf_font_t *font;
bdf_callback_t callback;
#endif
{
	return _bdf_set_glyph_names(in, font, callback, 0);
}

int
#ifdef __STDC__
bdf_set_adobe_glyph_names(FILE *in, bdf_font_t *font, bdf_callback_t callback)
#else
bdf_set_adobe_glyph_names(in, font, callback)
FILE *in;
bdf_font_t *font;
bdf_callback_t callback;
#endif
{
	return _bdf_set_glyph_names(in, font, callback, 1);
}

int
#ifdef __STDC__
bdf_set_glyph_code_names(int prefix, bdf_font_t *font, bdf_callback_t callback)
#else
bdf_set_glyph_code_names(prefix, font, callback)
int prefix;
bdf_font_t *font;
bdf_callback_t callback;
#endif
{
	int changed;
	long i, size, len;
	bdf_glyph_t *gp;
	bdf_callback_struct_t cb;
	char name[128];

	if (callback != 0) {
		cb.reason = BDF_GLYPH_NAME_START;
		cb.current = 0;
		cb.total = font->glyphs_used;
		(*callback)(&cb, 0);
	}
	for (changed = 0, i = 0, gp = font->glyphs; i < font->glyphs_used;
	        i++, gp++) {
		switch (prefix) {
		case 'u':
			sprintf(name, "uni%04lX", gp->encoding & 0xffff);
			break;
		case 'x':
			sprintf(name, "0x%04lX", gp->encoding & 0xffff);
			break;
		case '+':
			sprintf(name, "U+%04lX", gp->encoding & 0xffff);
			break;
		case '\\':
			sprintf(name, "\\u%04lX", gp->encoding & 0xffff);
			break;
		}
		size = 6;

		len = (gp->name) ? strlen(gp->name) : 0;
		if (len == 0) {
			gp->name = (char *) _bdf_strdup((unsigned char *) name, size + 1);
			changed = 1;
		} else if (size != len || strcmp(gp->name, name) != 0) {
			/*
			 * Simply resize existing storage so lots of memory allocations
			 * are not needed.
			 */
			if (size > len)
				gp->name = (char *) realloc(gp->name, size + 1);
			(void) strcpy(gp->name, name);
			changed = 1;
		}

		if (callback != 0) {
			cb.reason = BDF_GLYPH_NAME;
			cb.current = i;
			(*callback)(&cb, 0);
		}
	}

	if (callback != 0) {
		cb.reason = BDF_GLYPH_NAME;
		cb.current = cb.total;
		(*callback)(&cb, 0);
	}

	return changed;
}

void
#ifdef __STDC__
_bdf_glyph_name_cleanup(void)
#else
_bdf_glyph_name_cleanup()
#endif
{
	if (adobe_names_size > 0)
		free((char *) adobe_names);
	adobe_names_size = adobe_names_used = 0;
}
