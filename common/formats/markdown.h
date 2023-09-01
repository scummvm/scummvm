/*
 * Copyright (c) 2009, Natacha Porté
 * Copyright (c) 2011, Vicent Marti
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef COMMON_FORMATS_MARKDOWN_H
#define COMMON_FORMATS_MARKDOWN_H

#include "common/scummsys.h"

namespace Common {

#define SUNDOWN_VERSION "1.16.0"
#define SUNDOWN_VER_MAJOR 1
#define SUNDOWN_VER_MINOR 16
#define SUNDOWN_VER_REVISION 0

/********************
 * TYPE DEFINITIONS *
 ********************/

struct DataBuffer;
struct SDMarkdown;

/* mkd_autolink - type of autolink */
enum MKDAutolink {
	MKDA_NOT_AUTOLINK,	/* used internally when it is not an autolink*/
	MKDA_NORMAL,		/* normal http/http/ftp/mailto/etc link */
	MKDA_EMAIL,			/* e-mail link without explit mailto: */
};

enum mkd_tableflags {
	MKD_TABLE_ALIGN_L = 1,
	MKD_TABLE_ALIGN_R = 2,
	MKD_TABLE_ALIGN_CENTER = 3,
	MKD_TABLE_ALIGNMASK = 3,
	MKD_TABLE_HEADER = 4
};

enum mkd_extensions {
	MKDEXT_NO_INTRA_EMPHASIS = (1 << 0),
	MKDEXT_TABLES = (1 << 1),
	MKDEXT_FENCED_CODE = (1 << 2),
	MKDEXT_AUTOLINK = (1 << 3),
	MKDEXT_STRIKETHROUGH = (1 << 4),
	MKDEXT_SPACE_HEADERS = (1 << 6),
	MKDEXT_SUPERSCRIPT = (1 << 7),
	MKDEXT_LAX_SPACING = (1 << 8),
};

/* sd_callbacks - functions for rendering parsed data */
struct SDCallbacks {
	/* block level callbacks - NULL skips the block */
	void (*blockcode)(DataBuffer *ob, const DataBuffer *text, const DataBuffer *lang, void *opaque);
	void (*blockquote)(DataBuffer *ob, const DataBuffer *text, void *opaque);
	void (*blockhtml)(DataBuffer *ob,const  DataBuffer *text, void *opaque);
	void (*header)(DataBuffer *ob, const DataBuffer *text, int level, void *opaque);
	void (*hrule)(DataBuffer *ob, void *opaque);
	void (*list_start)(DataBuffer *ob, const DataBuffer *text, int flags, void *opaque);
	void (*list)(DataBuffer *ob, const DataBuffer *text, int flags, void *opaque);
	void (*listitem)(DataBuffer *ob, const DataBuffer *text, int flags, void *opaque);
	void (*paragraph)(DataBuffer *ob, const DataBuffer *text, void *opaque);
	void (*table)(DataBuffer *ob, const DataBuffer *header, const DataBuffer *body, void *opaque);
	void (*table_row)(DataBuffer *ob, const DataBuffer *text, void *opaque);
	void (*table_cell)(DataBuffer *ob, const DataBuffer *text, int flags, void *opaque);


	/* span level callbacks - NULL or return 0 prints the span verbatim */
	int (*autolink)(DataBuffer *ob, const DataBuffer *link, MKDAutolink type, void *opaque);
	int (*codespan)(DataBuffer *ob, const DataBuffer *text, void *opaque);
	int (*double_emphasis)(DataBuffer *ob, const DataBuffer *text, void *opaque);
	int (*emphasis)(DataBuffer *ob, const DataBuffer *text, void *opaque);
	int (*image)(DataBuffer *ob, const DataBuffer *link, const DataBuffer *title, const DataBuffer *alt, void *opaque);
	int (*linebreak)(DataBuffer *ob, void *opaque);
	int (*link)(DataBuffer *ob, const DataBuffer *link, const DataBuffer *title, const DataBuffer *content, void *opaque);
	int (*raw_html_tag)(DataBuffer *ob, const DataBuffer *tag, void *opaque);
	int (*triple_emphasis)(DataBuffer *ob, const DataBuffer *text, void *opaque);
	int (*strikethrough)(DataBuffer *ob, const DataBuffer *text, void *opaque);
	int (*superscript)(DataBuffer *ob, const DataBuffer *text, void *opaque);

	/* low level callbacks - NULL copies input directly into the output */
	void (*entity)(DataBuffer *ob, const DataBuffer *entity, void *opaque);
	void (*normal_text)(DataBuffer *ob, const DataBuffer *text, void *opaque);

	/* header and footer */
	void (*doc_header)(DataBuffer *ob, void *opaque);
	void (*doc_footer)(DataBuffer *ob, void *opaque);
};

/*********
 * FLAGS *
 *********/

/* list/listitem flags */
#define MKD_LIST_ORDERED	1
#define MKD_LI_BLOCK		2  /* <li> containing block data */

SDMarkdown *sd_markdown_new(uint extensions, size_t max_nesting, const SDCallbacks *callbacks, void *opaque);

void sd_markdown_render(DataBuffer *ob, const byte *document, size_t doc_size, SDMarkdown *md);

void sd_markdown_free(SDMarkdown *md);

void sd_version(int *major, int *minor, int *revision);

// buffer.h

enum buferror_t {
	BUF_OK = 0,
	BUF_ENOMEM = -1,
};

/* DataBuffer: character array buffer */
struct DataBuffer {
	byte *data;		/* actual character data */
	size_t size;	/* size of the string */
	size_t asize;	/* allocated size (0 = volatile buffer) */
	size_t unit;	/* reallocation unit size (0 = read-only buffer) */
};

/* CONST_BUF: global buffer from a string litteral */
#define BUF_STATIC(string) \
	{ (byte *)string, sizeof string -1, sizeof string, 0, 0 }

/* VOLATILE_BUF: macro for creating a volatile buffer on the stack */
#define BUF_VOLATILE(strname) \
	{ (byte *)strname, strlen(strname), 0, 0, 0 }

/* BUFPUTSL: optimized bufputs of a string litteral */
#define BUFPUTSL(output, literal) \
	bufput(output, literal, sizeof literal - 1)

/* bufgrow: increasing the allocated size to the given value */
int bufgrow(DataBuffer *, size_t);

/* bufnew: allocation of a new buffer */
DataBuffer *bufnew(size_t);

/* bufput: appends raw data to a buffer */
void bufput(DataBuffer *, const void *, size_t);

/* bufputc: appends a single char to a buffer */
void bufputc(DataBuffer *, int);

/* bufrelease: decrease the reference count and free the buffer if needed */
void bufrelease(DataBuffer *);

} // end of namespace Common

#endif // COMMON_FORMATS_MARKDOWN_H
