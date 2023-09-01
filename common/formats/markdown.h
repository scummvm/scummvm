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

class String;

#define SUNDOWN_VERSION "1.16.0"
#define SUNDOWN_VER_MAJOR 1
#define SUNDOWN_VER_MINOR 16
#define SUNDOWN_VER_REVISION 0

/********************
 * TYPE DEFINITIONS *
 ********************/

struct SDDataBuffer;

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

struct SDStack {
	void **item;
	size_t size;
	size_t asize;
};

/* sd_callbacks - functions for rendering parsed data */
struct SDCallbacks {
	/* block level callbacks - NULL skips the block */
	void (*blockcode)(SDDataBuffer *ob, const SDDataBuffer *text, const SDDataBuffer *lang, void *opaque);
	void (*blockquote)(SDDataBuffer *ob, const SDDataBuffer *text, void *opaque);
	void (*blockhtml)(SDDataBuffer *ob,const  SDDataBuffer *text, void *opaque);
	void (*header)(SDDataBuffer *ob, const SDDataBuffer *text, int level, void *opaque);
	void (*hrule)(SDDataBuffer *ob, void *opaque);
	void (*list_start)(SDDataBuffer *ob, const SDDataBuffer *text, int flags, void *opaque);
	void (*list)(SDDataBuffer *ob, const SDDataBuffer *text, int flags, void *opaque);
	void (*listitem)(SDDataBuffer *ob, const SDDataBuffer *text, int flags, void *opaque);
	void (*paragraph)(SDDataBuffer *ob, const SDDataBuffer *text, void *opaque);
	void (*table)(SDDataBuffer *ob, const SDDataBuffer *header, const SDDataBuffer *body, void *opaque);
	void (*table_row)(SDDataBuffer *ob, const SDDataBuffer *text, void *opaque);
	void (*table_cell)(SDDataBuffer *ob, const SDDataBuffer *text, int flags, void *opaque);


	/* span level callbacks - NULL or return 0 prints the span verbatim */
	int (*autolink)(SDDataBuffer *ob, const SDDataBuffer *link, MKDAutolink type, void *opaque);
	int (*codespan)(SDDataBuffer *ob, const SDDataBuffer *text, void *opaque);
	int (*double_emphasis)(SDDataBuffer *ob, const SDDataBuffer *text, void *opaque);
	int (*emphasis)(SDDataBuffer *ob, const SDDataBuffer *text, void *opaque);
	int (*image)(SDDataBuffer *ob, const SDDataBuffer *link, const SDDataBuffer *title, const SDDataBuffer *alt, void *opaque);
	int (*linebreak)(SDDataBuffer *ob, void *opaque);
	int (*link)(SDDataBuffer *ob, const SDDataBuffer *link, const SDDataBuffer *title, const SDDataBuffer *content, void *opaque);
	int (*raw_html_tag)(SDDataBuffer *ob, const SDDataBuffer *tag, void *opaque);
	int (*triple_emphasis)(SDDataBuffer *ob, const SDDataBuffer *text, void *opaque);
	int (*strikethrough)(SDDataBuffer *ob, const SDDataBuffer *text, void *opaque);
	int (*superscript)(SDDataBuffer *ob, const SDDataBuffer *text, void *opaque);

	/* low level callbacks - NULL copies input directly into the output */
	void (*entity)(SDDataBuffer *ob, const SDDataBuffer *entity, void *opaque);
	void (*normal_text)(SDDataBuffer *ob, const SDDataBuffer *text, void *opaque);

	/* header and footer */
	void (*doc_header)(SDDataBuffer *ob, void *opaque);
	void (*doc_footer)(SDDataBuffer *ob, void *opaque);
};

/*********
 * FLAGS *
 *********/

/* list/listitem flags */
#define MKD_LIST_ORDERED	1
#define MKD_LI_BLOCK		2  /* <li> containing block data */

#define REF_TABLE_SIZE 8

struct LinkRef;

class SDMarkdown {
public:
	SDCallbacks _cb;
	void *_opaque;

	LinkRef *_refs[REF_TABLE_SIZE];
	byte _active_char[256];
	SDStack _work_bufs[2];
	uint _ext_flags;
	size_t _max_nesting;
	int _in_link_body;

	SDMarkdown(uint extensions, size_t max_nesting, const SDCallbacks *callbacks, void *opaque);
	~SDMarkdown();

	Common::String render(const byte *document, size_t doc_size);

	void version(int *major, int *minor, int *revision);
};

/* SDDataBuffer: character array buffer */
struct SDDataBuffer {
	byte *data;		/* actual character data */
	size_t size;	/* size of the string */
	size_t asize;	/* allocated size (0 = volatile buffer) */
	size_t unit;	/* reallocation unit size (0 = read-only buffer) */
};

/* sd_bufput: appends raw data to a buffer */
void sd_bufput(SDDataBuffer *, const void *, size_t);

} // end of namespace Common

#endif // COMMON_FORMATS_MARKDOWN_H
