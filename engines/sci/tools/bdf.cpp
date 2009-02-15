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
static char rcsid[] __attribute__((unused)) = "$Id: bdf.c 1284 2004-04-02 07:42:44Z jameson $";
#else
static char rcsid[] = "$Id: bdf.c 1284 2004-04-02 07:42:44Z jameson $";
#endif
#endif

#include "bdfP.h"

#ifdef HAVE_HBF
#include "hbf.h"
#endif

#undef MAX
#define MAX(h, i) ((h) > (i) ? (h) : (i))

#undef MIN
#define MIN(l, o) ((l) < (o) ? (l) : (o))

/**************************************************************************
 *
 * Masks used for checking different bits per pixel cases.
 *
 **************************************************************************/

unsigned char onebpp[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
unsigned char twobpp[] = { 0xc0, 0x30, 0x0c, 0x03 };
unsigned char fourbpp[] = { 0xf0, 0x0f };
unsigned char eightbpp[] = { 0xff };

/**************************************************************************
 *
 * Default BDF font options.
 *
 **************************************************************************/

static bdf_options_t _bdf_opts = {
	1,                /* Hint TTF glyphs.               */
	1,                /* Correct metrics.               */
	1,                /* Preserve unencoded glyphs.     */
	1,                /* Preserve comments.             */
	1,                /* Pad character-cells.           */
	BDF_PROPORTIONAL, /* Default spacing.               */
	12,               /* Default point size.            */
	0,                /* Default horizontal resolution. */
	0,                /* Default vertical resolution.   */
	1,                /* Bits per pixel.                */
	BDF_UNIX_EOL,     /* Line separator.                */
};

/**************************************************************************
 *
 * Builtin BDF font properties.
 *
 **************************************************************************/

/*
 * List of most properties that might appear in a font.  Doesn't include the
 * RAW_* and AXIS_* properties in X11R6 polymorphic fonts.
 */
static bdf_property_t _bdf_properties[] = {
	{"ADD_STYLE_NAME",          BDF_ATOM,     1},
	{"AVERAGE_WIDTH",           BDF_INTEGER,  1},
	{"AVG_CAPITAL_WIDTH",       BDF_INTEGER,  1},
	{"AVG_LOWERCASE_WIDTH",     BDF_INTEGER,  1},
	{"CAP_HEIGHT",              BDF_INTEGER,  1},
	{"CHARSET_COLLECTIONS",     BDF_ATOM,     1},
	{"CHARSET_ENCODING",        BDF_ATOM,     1},
	{"CHARSET_REGISTRY",        BDF_ATOM,     1},
	{"COMMENT",                 BDF_ATOM,     1},
	{"COPYRIGHT",               BDF_ATOM,     1},
	{"DEFAULT_CHAR",            BDF_CARDINAL, 1},
	{"DESTINATION",             BDF_CARDINAL, 1},
	{"DEVICE_FONT_NAME",        BDF_ATOM,     1},
	{"END_SPACE",               BDF_INTEGER,  1},
	{"FACE_NAME",               BDF_ATOM,     1},
	{"FAMILY_NAME",             BDF_ATOM,     1},
	{"FIGURE_WIDTH",            BDF_INTEGER,  1},
	{"FONT",                    BDF_ATOM,     1},
	{"FONTNAME_REGISTRY",       BDF_ATOM,     1},
	{"FONT_ASCENT",             BDF_INTEGER,  1},
	{"FONT_DESCENT",            BDF_INTEGER,  1},
	{"FOUNDRY",                 BDF_ATOM,     1},
	{"FULL_NAME",               BDF_ATOM,     1},
	{"ITALIC_ANGLE",            BDF_INTEGER,  1},
	{"MAX_SPACE",               BDF_INTEGER,  1},
	{"MIN_SPACE",               BDF_INTEGER,  1},
	{"NORM_SPACE",              BDF_INTEGER,  1},
	{"NOTICE",                  BDF_ATOM,     1},
	{"PIXEL_SIZE",              BDF_INTEGER,  1},
	{"POINT_SIZE",              BDF_INTEGER,  1},
	{"QUAD_WIDTH",              BDF_INTEGER,  1},
	{"RAW_ASCENT",              BDF_INTEGER,  1},
	{"RAW_AVERAGE_WIDTH",       BDF_INTEGER,  1},
	{"RAW_AVG_CAPITAL_WIDTH",   BDF_INTEGER,  1},
	{"RAW_AVG_LOWERCASE_WIDTH", BDF_INTEGER,  1},
	{"RAW_CAP_HEIGHT",          BDF_INTEGER,  1},
	{"RAW_DESCENT",             BDF_INTEGER,  1},
	{"RAW_END_SPACE",           BDF_INTEGER,  1},
	{"RAW_FIGURE_WIDTH",        BDF_INTEGER,  1},
	{"RAW_MAX_SPACE",           BDF_INTEGER,  1},
	{"RAW_MIN_SPACE",           BDF_INTEGER,  1},
	{"RAW_NORM_SPACE",          BDF_INTEGER,  1},
	{"RAW_PIXEL_SIZE",          BDF_INTEGER,  1},
	{"RAW_POINT_SIZE",          BDF_INTEGER,  1},
	{"RAW_PIXELSIZE",           BDF_INTEGER,  1},
	{"RAW_POINTSIZE",           BDF_INTEGER,  1},
	{"RAW_QUAD_WIDTH",          BDF_INTEGER,  1},
	{"RAW_SMALL_CAP_SIZE",      BDF_INTEGER,  1},
	{"RAW_STRIKEOUT_ASCENT",    BDF_INTEGER,  1},
	{"RAW_STRIKEOUT_DESCENT",   BDF_INTEGER,  1},
	{"RAW_SUBSCRIPT_SIZE",      BDF_INTEGER,  1},
	{"RAW_SUBSCRIPT_X",         BDF_INTEGER,  1},
	{"RAW_SUBSCRIPT_Y",         BDF_INTEGER,  1},
	{"RAW_SUPERSCRIPT_SIZE",    BDF_INTEGER,  1},
	{"RAW_SUPERSCRIPT_X",       BDF_INTEGER,  1},
	{"RAW_SUPERSCRIPT_Y",       BDF_INTEGER,  1},
	{"RAW_UNDERLINE_POSITION",  BDF_INTEGER,  1},
	{"RAW_UNDERLINE_THICKNESS", BDF_INTEGER,  1},
	{"RAW_X_HEIGHT",            BDF_INTEGER,  1},
	{"RELATIVE_SETWIDTH",       BDF_CARDINAL, 1},
	{"RELATIVE_WEIGHT",         BDF_CARDINAL, 1},
	{"RESOLUTION",              BDF_INTEGER,  1},
	{"RESOLUTION_X",            BDF_CARDINAL, 1},
	{"RESOLUTION_Y",            BDF_CARDINAL, 1},
	{"SETWIDTH_NAME",           BDF_ATOM,     1},
	{"SLANT",                   BDF_ATOM,     1},
	{"SMALL_CAP_SIZE",          BDF_INTEGER,  1},
	{"SPACING",                 BDF_ATOM,     1},
	{"STRIKEOUT_ASCENT",        BDF_INTEGER,  1},
	{"STRIKEOUT_DESCENT",       BDF_INTEGER,  1},
	{"SUBSCRIPT_SIZE",          BDF_INTEGER,  1},
	{"SUBSCRIPT_X",             BDF_INTEGER,  1},
	{"SUBSCRIPT_Y",             BDF_INTEGER,  1},
	{"SUPERSCRIPT_SIZE",        BDF_INTEGER,  1},
	{"SUPERSCRIPT_X",           BDF_INTEGER,  1},
	{"SUPERSCRIPT_Y",           BDF_INTEGER,  1},
	{"UNDERLINE_POSITION",      BDF_INTEGER,  1},
	{"UNDERLINE_THICKNESS",     BDF_INTEGER,  1},
	{"WEIGHT",                  BDF_CARDINAL, 1},
	{"WEIGHT_NAME",             BDF_ATOM,     1},
	{"X_HEIGHT",                BDF_INTEGER,  1},
	{"_MULE_BASELINE_OFFSET",   BDF_INTEGER,  1},
	{"_MULE_RELATIVE_COMPOSE",  BDF_INTEGER,  1},
};

static unsigned long _num_bdf_properties =
    sizeof(_bdf_properties) / sizeof(_bdf_properties[0]);

/*
 * User defined properties.
 */
static bdf_property_t *user_props;
static unsigned long nuser_props = 0;

/**************************************************************************
 *
 * Hash table utilities for the properties.
 *
 **************************************************************************/

#define INITIAL_HT_SIZE 241

typedef struct {
	char *key;
	void *data;
} _hashnode, *hashnode;

typedef struct {
	int limit;
	int size;
	int used;
	hashnode *table;
} hashtable;

typedef void (*hash_free_func)(
#ifdef __STDC__
    hashnode node
#endif
);

static hashnode *
#ifdef __STDC__
hash_bucket(char *key, hashtable *ht)
#else
hash_bucket(key, ht)
char *key;
hashtable *ht;
#endif
{
	char *kp = key;
	unsigned long res = 0;
	hashnode *bp = ht->table, *ndp;

	/*
	 * Mocklisp hash function.
	 */
	while (*kp)
		res = (res << 5) - res + *kp++;

	ndp = bp + (res % ht->size);
	while (*ndp) {
		kp = (*ndp)->key;
		if (kp[0] == key[0] && strcmp(kp, key) == 0)
			break;
		ndp--;
		if (ndp < bp)
			ndp = bp + (ht->size - 1);
	}
	return ndp;
}

static void
#ifdef __STDC__
hash_rehash(hashtable *ht)
#else
hash_rehash(ht)
hashtable *ht;
#endif
{
	hashnode *obp = ht->table, *bp, *nbp;
	int i, sz = ht->size;

	ht->size <<= 1;
	ht->limit = ht->size / 3;
	ht->table = (hashnode *) malloc(sizeof(hashnode) * ht->size);
	(void) memset((char *) ht->table, 0, sizeof(hashnode) * ht->size);

	for (i = 0, bp = obp; i < sz; i++, bp++) {
		if (*bp) {
			nbp = hash_bucket((*bp)->key, ht);
			*nbp = *bp;
		}
	}
	free((char *) obp);
}

static void
#ifdef __STDC__
hash_init(hashtable *ht)
#else
hash_init(ht)
hashtable *ht;
#endif
{
	int sz = INITIAL_HT_SIZE;

	ht->size = sz;
	ht->limit = sz / 3;
	ht->used = 0;
	ht->table = (hashnode *) malloc(sizeof(hashnode) * sz);
	(void) memset((char *) ht->table, 0, sizeof(hashnode) * sz);
}

static void
#ifdef __STDC__
hash_free(hashtable *ht)
#else
hash_free(ht)
hashtable *ht;
#endif
{
	int i, sz = ht->size;
	hashnode *bp = ht->table;

	for (i = 0; i < sz; i++, bp++) {
		if (*bp)
			free((char *) *bp);
	}
	if (sz > 0)
		free((char *) ht->table);
}

static void
#ifdef __STDC__
hash_insert(char *key, void *data, hashtable *ht)
#else
hash_insert(key, data, ht)
char *key;
void *data;
hashtable *ht;
#endif
{
	hashnode nn, *bp = hash_bucket(key, ht);

	nn = *bp;
	if (!nn) {
		*bp = nn = (hashnode) malloc(sizeof(_hashnode));
		nn->key = key;
		nn->data = data;

		if (ht->used >= ht->limit)
			hash_rehash(ht);
		ht->used++;
	} else
		nn->data = data;
}

static hashnode
#ifdef __STDC__
hash_lookup(char *key, hashtable *ht)
#else
hash_lookup(key, ht)
char *key;
hashtable *ht;
#endif
{
	hashnode *np = hash_bucket(key, ht);
	return *np;
}

static void
#ifdef __STDC__
hash_delete(char *name, hashtable *ht)
#else
hash_delete(name, ht)
char *name;
hashtable *ht;
#endif
{
	hashnode *hp;

	hp = hash_bucket(name, ht);
	if (*hp) {
		free((char *) *hp);
		*hp = 0;
	}
}

/*
 * The builtin property table.
 */
static hashtable proptbl;

/**************************************************************************
 *
 * Utility types and functions.
 *
 **************************************************************************/

/*
 * Function type for parsing lines of a BDF font.
 */
typedef int (*_bdf_line_func_t)(
#ifdef __STDC__
    char *line,
    unsigned long linelen,
    unsigned long lineno,
    void *call_data,
    void *client_data
#endif
);

/*
 * List structure for splitting lines into fields.
 */
typedef struct {
	char **field;
	unsigned long size;
	unsigned long used;
	char *bfield;
	unsigned long bsize;
	unsigned long bused;
} _bdf_list_t;

/*
 * Structure used while loading BDF fonts.
 */
typedef struct {
	unsigned long flags;
	unsigned long cnt;
	unsigned long row;
	unsigned long bpr;
	short minlb;
	short maxlb;
	short maxrb;
	short maxas;
	short maxds;
	short rbearing;
	char *glyph_name;
	long glyph_enc;
	bdf_font_t *font;
	bdf_options_t *opts;
	void *client_data;
	bdf_callback_t callback;
	bdf_callback_struct_t cb;
	unsigned long have[2048];
	_bdf_list_t list;
} _bdf_parse_t;

#define setsbit(m, cc) (m[(cc) >> 3] |= (1 << ((cc) & 7)))
#define sbitset(m, cc) (m[(cc) >> 3] & (1 << ((cc) & 7)))

/*
 * An empty string for empty fields.
 */
static char empty[1] = { 0 };

/*
 * Assume the line is NULL terminated and that the `list' parameter was
 * initialized the first time it was used.
 */
static void
#ifdef __STDC__
_bdf_split(char *separators, char *line, unsigned long linelen,
           _bdf_list_t *list)
#else
_bdf_split(separators, line, linelen, list)
char *separators, *line;
unsigned long linelen;
_bdf_list_t *list;
#endif
{
	int mult, final_empty;
	char *sp, *ep, *end;
	unsigned char seps[32];

	/*
	 * Initialize the list.
	 */
	list->used = list->bused = 0;

	/*
	 * If the line is empty, then simply return.
	 */
	if (linelen == 0 || line[0] == 0)
		return;

	/*
	 * If the `separators' parameter is NULL or empty, split the list into
	 * individual bytes.
	 */
	if (separators == 0 || *separators == 0) {
		if (linelen > list->bsize) {
			if (list->bsize)
				list->bfield = (char *) malloc(linelen);
			else
				list->bfield = (char *) realloc(list->bfield, linelen);
			list->bsize = linelen;
		}
		list->bused = linelen;
		(void) memcpy(list->bfield, line, linelen);
		return;
	}

	/*
	 * Prepare the separator bitmap.
	 */
	(void) memset((char *) seps, 0, 32);

	/*
	 * If the very last character of the separator string is a plus, then set
	 * the `mult' flag to indicate that multiple separators should be
	 * collapsed into one.
	 */
	for (mult = 0, sp = separators; sp && *sp; sp++) {
		if (*sp == '+' && *(sp + 1) == 0)
			mult = 1;
		else
			setsbit(seps, *sp);
	}

	/*
	 * Break the line up into fields.
	 */
	for (final_empty = 0, sp = ep = line, end = sp + linelen;
	        sp < end && *sp;) {
		/*
		 * Collect everything that is not a separator.
		 */
		for (; *ep && !sbitset(seps, *ep); ep++) ;

		/*
		 * Resize the list if necessary.
		 */
		if (list->used == list->size) {
			if (list->size == 0)
				list->field = (char **) malloc(sizeof(char *) * 5);
			else
				list->field = (char **)
				              realloc((char *) list->field,
				                      sizeof(char *) * (list->size + 5));

			list->size += 5;
		}

		/*
		 * Assign the field appropriately.
		 */
		list->field[list->used++] = (ep > sp) ? sp : empty;

		sp = ep;
		if (mult) {
			/*
			 * If multiple separators should be collapsed, do it now by
			 * setting all the separator characters to 0.
			 */
			for (; *ep && sbitset(seps, *ep); ep++)
				*ep = 0;
		} else if (*ep != 0)
			/*
			 * Don't collapse multiple separators by making them 0, so just
			 * make the one encountered 0.
			 */
			*ep++ = 0;
		final_empty = (ep > sp && *ep == 0);
		sp = ep;
	}

	/*
	 * Finally, NULL terminate the list.
	 */
	if (list->used + final_empty + 1 >= list->size) {
		if (list->used == list->size) {
			if (list->size == 0)
				list->field = (char **) malloc(sizeof(char *) * 5);
			else
				list->field = (char **)
				              realloc((char *) list->field,
				                      sizeof(char *) * (list->size + 5));
			list->size += 5;
		}
	}
	if (final_empty)
		list->field[list->used++] = empty;

	if (list->used == list->size) {
		if (list->size == 0)
			list->field = (char **) malloc(sizeof(char *) * 5);
		else
			list->field = (char **)
			              realloc((char *) list->field,
			                      sizeof(char *) * (list->size + 5));
		list->size += 5;
	}
	list->field[list->used] = 0;
}

static void
#ifdef __STDC__
_bdf_shift(unsigned long n, _bdf_list_t *list)
#else
_bdf_shift(n, list)
unsigned long n;
_bdf_list_t *list;
#endif
{
	unsigned long i, u;

	if (list == 0 || list->used == 0 || n == 0)
		return;

	if (n >= list->used) {
		list->used = 0;
		return;
	}
	for (u = n, i = 0; u < list->used; i++, u++)
		list->field[i] = list->field[u];
	list->used -= n;
}

static char *
#ifdef __STDC__
_bdf_join(int c, unsigned long *len, _bdf_list_t *list)
#else
_bdf_join(c, len, list)
int c;
unsigned long *len;
_bdf_list_t *list;
#endif
{
	unsigned long i, j;
	char *fp, *dp;

	if (list == 0 || list->used == 0)
		return 0;

	*len = 0;

	dp = list->field[0];
	for (i = j = 0; i < list->used; i++) {
		fp = list->field[i];
		while (*fp)
			dp[j++] = *fp++;
		if (i + 1 < list->used)
			dp[j++] = c;
	}
	dp[j] = 0;

	*len = j;
	return dp;
}

/*
 * High speed file reader that passes each line to a callback.
 */
static int
#ifdef __STDC__
_bdf_readlines(int fd, _bdf_line_func_t callback, void *client_data,
               unsigned long *lno)
#else
_bdf_readlines(fd, callback, client_data, lno)
int fd;
_bdf_line_func_t callback;
void *client_data;
unsigned long *lno;
#endif
{
	_bdf_line_func_t cb;
	unsigned long lineno;
	int n, res, done, refill, bytes, hold;
	char *ls, *le, *pp, *pe, *hp;
	char buf[65536];

	if (callback == 0)
		return -1;

	cb = callback;
	lineno = 1;
	buf[0] = 0;
	res = done = 0;
	pp = ls = le = buf;
	bytes = 65536;
	while (!done && (n = read(fd, pp, bytes)) > 0) {
		/*
		 * Determine the new end of the buffer pages.
		 */
		pe = pp + n;

		for (refill = 0; done == 0 && refill == 0;) {
			while (le < pe && *le != '\n' && *le != '\r')
				le++;

			if (le == pe) {
				/*
				 * Hit the end of the last page in the buffer.  Need to find
				 * out how many pages to shift and how many pages need to be
				 * read in.  Adjust the line start and end pointers down to
				 * point to the right places in the pages.
				 */
				pp = buf + (((ls - buf) >> 13) << 13);
				n = pp - buf;
				ls -= n;
				le -= n;
				n = pe - pp;
				(void) memcpy(buf, pp, n);
				pp = buf + n;
				bytes = 65536 - n;
				refill = 1;
			} else {
				/*
				 * Temporarily NULL terminate the line.
				 */
				hp = le;
				hold = *le;
				*le = 0;

				if (callback && *ls != '#' && *ls != 0x1a && le > ls &&
				        (res = (*cb)(ls, le - ls, lineno, (void *) & cb,
				                     client_data)) != 0)
					done = 1;
				else {
					ls = ++le;
					/*
					 * Handle the case of DOS crlf sequences.
					 */
					if (le < pe && hold == '\n' && *le == '\r')
						ls = ++le;
				}

				/*
				 * Increment the line number.
				 */
				lineno++;

				/*
				 * Restore the character at the end of the line.
				 */
				*hp = hold;
			}
		}
	}
	*lno = lineno;
	return res;
}

unsigned char *
#ifdef __STDC__
_bdf_strdup(unsigned char *s, unsigned long len)
#else
_bdf_strdup(s, len)
unsigned char *s;
unsigned long len;
#endif
{
	unsigned char *ns;

	if (s == 0 || len == 0)
		return 0;

	ns = (unsigned char *) malloc(len);
	(void) memcpy((char *) ns, (char *) s, len);
	return ns;
}

void
_bdf_memmove(char *dest, char *src, unsigned long bytes) {
	long i, j;

	i = (long) bytes;
	j = i & 7;
	i = (i + 7) >> 3;

	/*
	 * Do a memmove using Ye Olde Duff's Device for efficiency.
	 */
	if (src < dest) {
		src += bytes;
		dest += bytes;

		switch (j) {
		case 0:
			do {
				*--dest = *--src;
			case 7:
				*--dest = *--src;
			case 6:
				*--dest = *--src;
			case 5:
				*--dest = *--src;
			case 4:
				*--dest = *--src;
			case 3:
				*--dest = *--src;
			case 2:
				*--dest = *--src;
			case 1:
				*--dest = *--src;
			} while (--i > 0);
		}
	} else if (src > dest) {
		switch (j) {
		case 0:
			do {
				*dest++ = *src++;
			case 7:
				*dest++ = *src++;
			case 6:
				*dest++ = *src++;
			case 5:
				*dest++ = *src++;
			case 4:
				*dest++ = *src++;
			case 3:
				*dest++ = *src++;
			case 2:
				*dest++ = *src++;
			case 1:
				*dest++ = *src++;
			} while (--i > 0);
		}
	}
}

static unsigned char a2i[128] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static unsigned char odigits[32] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static unsigned char ddigits[32] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x03,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static unsigned char hdigits[32] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x03,
	0x7e, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

#define isdigok(m, d) (m[(d) >> 3] & (1 << ((d) & 7)))

/*
 * Routine to convert an ASCII string into an unsigned long integer.
 */
unsigned long
#ifdef __STDC__
_bdf_atoul(char *s, char **end, int base)
#else
_bdf_atoul(s, end, base)
char *s, **end;
int base;
#endif
{
	unsigned long v;
	unsigned char *dmap;

	if (s == 0 || *s == 0)
		return 0;

	/*
	 * Make sure the radix is something recognizable.  Default to 10.
	 */
	switch (base) {
	case 8:
		dmap = odigits;
		break;
	case 16:
		dmap = hdigits;
		break;
	default:
		base = 10;
		dmap = ddigits;
		break;
	}

	/*
	 * Check for the special hex prefix.
	 */
	if (*s == '0' && (*(s + 1) == 'x' || *(s + 1) == 'X')) {
		base = 16;
		dmap = hdigits;
		s += 2;
	}

	for (v = 0; isdigok(dmap, *s); s++)
		v = (v * base) + a2i[(int) *s];

	if (end != 0)
		*end = s;

	return v;
}

/*
 * Routine to convert an ASCII string into an signed long integer.
 */
long
#ifdef __STDC__
_bdf_atol(char *s, char **end, int base)
#else
_bdf_atol(s, end, base)
char *s, **end;
int base;
#endif
{
	long v, neg;
	unsigned char *dmap;

	if (s == 0 || *s == 0)
		return 0;

	/*
	 * Make sure the radix is something recognizable.  Default to 10.
	 */
	switch (base) {
	case 8:
		dmap = odigits;
		break;
	case 16:
		dmap = hdigits;
		break;
	default:
		base = 10;
		dmap = ddigits;
		break;
	}

	/*
	 * Check for a minus sign.
	 */
	neg = 0;
	if (*s == '-') {
		s++;
		neg = 1;
	}

	/*
	 * Check for the special hex prefix.
	 */
	if (*s == '0' && (*(s + 1) == 'x' || *(s + 1) == 'X')) {
		base = 16;
		dmap = hdigits;
		s += 2;
	}

	for (v = 0; isdigok(dmap, *s); s++)
		v = (v * base) + a2i[(int) *s];

	if (end != 0)
		*end = s;

	return (!neg) ? v : -v;
}

/*
 * Routine to convert an ASCII string into an signed short integer.
 */
short
#ifdef __STDC__
_bdf_atos(char *s, char **end, int base)
#else
_bdf_atos(s, end, base)
char *s, **end;
int base;
#endif
{
	short v, neg;
	unsigned char *dmap;

	if (s == 0 || *s == 0)
		return 0;

	/*
	 * Make sure the radix is something recognizable.  Default to 10.
	 */
	switch (base) {
	case 8:
		dmap = odigits;
		break;
	case 16:
		dmap = hdigits;
		break;
	default:
		base = 10;
		dmap = ddigits;
		break;
	}

	/*
	 * Check for a minus.
	 */
	neg = 0;
	if (*s == '-') {
		s++;
		neg = 1;
	}

	/*
	 * Check for the special hex prefix.
	 */
	if (*s == '0' && (*(s + 1) == 'x' || *(s + 1) == 'X')) {
		base = 16;
		dmap = hdigits;
		s += 2;
	}

	for (v = 0; isdigok(dmap, *s); s++)
		v = (v * base) + a2i[(int) *s];

	if (end != 0)
		*end = s;

	return (!neg) ? v : -v;
}

/*
 * Routine to compare two glyphs by encoding so they can be sorted.
 */
static int
#ifdef __STDC__
by_encoding(const void *a, const void *b)
#else
by_encoding(a, b)
char *a, *b;
#endif
{
	bdf_glyph_t *c1, *c2;

	c1 = (bdf_glyph_t *) a;
	c2 = (bdf_glyph_t *) b;
	if (c1->encoding < c2->encoding)
		return -1;
	else if (c1->encoding > c2->encoding)
		return 1;
	return 0;
}

/**************************************************************************
 *
 * BDF font file parsing flags and functions.
 *
 **************************************************************************/

/*
 * Parse flags.
 */
#define _BDF_START     0x0001
#define _BDF_FONT_NAME 0x0002
#define _BDF_SIZE      0x0004
#define _BDF_FONT_BBX  0x0008
#define _BDF_PROPS     0x0010
#define _BDF_GLYPHS    0x0020
#define _BDF_GLYPH     0x0040
#define _BDF_ENCODING  0x0080
#define _BDF_SWIDTH    0x0100
#define _BDF_DWIDTH    0x0200
#define _BDF_BBX       0x0400
#define _BDF_BITMAP    0x0800

#define _BDF_SWIDTH_ADJ 0x1000

#define _BDF_GLYPH_BITS (_BDF_GLYPH|_BDF_ENCODING|_BDF_SWIDTH|\
                         _BDF_DWIDTH|_BDF_BBX|_BDF_BITMAP)

#define _BDF_GLYPH_WIDTH_CHECK 0x40000000
#define _BDF_GLYPH_HEIGHT_CHECK 0x80000000

/*
 * Auto correction messages.
 */
#define ACMSG1 "FONT_ASCENT property missing.  Added \"FONT_ASCENT %hd\"."
#define ACMSG2 "FONT_DESCENT property missing.  Added \"FONT_DESCENT %hd\"."
#define ACMSG3 "Font width != actual width.  Old: %hd New: %hd."
#define ACMSG4 "Font left bearing != actual left bearing.  Old: %hd New: %hd."
#define ACMSG5 "Font ascent != actual ascent.  Old: %hd New: %hd."
#define ACMSG6 "Font descent != actual descent.  Old: %hd New: %hd."
#define ACMSG7 "Font height != actual height. Old: %hd New: %hd."
#define ACMSG8 "Glyph scalable width (SWIDTH) adjustments made."
#define ACMSG9 "SWIDTH field missing at line %ld.  Set automatically."
#define ACMSG10 "DWIDTH field missing at line %ld.  Set to glyph width."
#define ACMSG11 "SIZE bits per pixel field adjusted to %hd."
#define ACMSG12 "Duplicate encoding %ld (%s) changed to unencoded."
#define ACMSG13 "Glyph %ld extra rows removed."
#define ACMSG14 "Glyph %ld extra columns removed."
#define ACMSG15 "Incorrect glyph count: %ld indicated but %ld found."

/*
 * Error messages.
 */
#define ERRMSG1 "[line %ld] Missing \"%s\" line."
#define ERRMSG2 "[line %ld] Font header corrupted or missing fields."
#define ERRMSG3 "[line %ld] Font glyphs corrupted or missing fields."

void
#ifdef __STDC__
_bdf_add_acmsg(bdf_font_t *font, char *msg, unsigned long len)
#else
_bdf_add_acmsg(font, msg, len)
bdf_font_t *font;
char *msg;
unsigned long len;
#endif
{
	char *cp;

	if (font->acmsgs_len == 0)
		font->acmsgs = (char *) malloc(len + 1);
	else
		font->acmsgs = (char *) realloc(font->acmsgs,
		                                font->acmsgs_len + len + 1);

	cp = font->acmsgs + font->acmsgs_len;
	(void) memcpy(cp, msg, len);
	cp += len;
	*cp++ = '\n';
	font->acmsgs_len += len + 1;
}

void
#ifdef __STDC__
_bdf_add_comment(bdf_font_t *font, char *comment, unsigned long len)
#else
_bdf_add_comment(font, comment, len)
bdf_font_t *font;
char *comment;
unsigned long len;
#endif
{
	char *cp;

	if (font->comments_len == 0)
		font->comments = (char *) malloc(len + 1);
	else
		font->comments = (char *) realloc(font->comments,
		                                  font->comments_len + len + 1);

	cp = font->comments + font->comments_len;
	(void) memcpy(cp, comment, len);
	cp += len;
	*cp++ = '\n';
	font->comments_len += len + 1;
}

/*
 * Set the spacing from the font name if it exists, or set it to the default
 * specified in the options.
 */
static void
#ifdef __STDC__
_bdf_set_default_spacing(bdf_font_t *font, bdf_options_t *opts)
#else
_bdf_set_default_spacing(font, opts)
bdf_font_t *font;
bdf_options_t *opts;
#endif
{
	unsigned long len;
	char name[128];
	_bdf_list_t list;

	if (font == 0 || font->name == 0 || font->name[0] == 0)
		return;

	font->spacing = opts->font_spacing;

	len = (unsigned long)(strlen(font->name) + 1);
	(void) memcpy(name, font->name, len);
	list.size = list.used = 0;
	_bdf_split("-", name, len, &list);
	if (list.used == 15) {
		switch (list.field[11][0]) {
		case 'C':
		case 'c':
			font->spacing = BDF_CHARCELL;
			break;
		case 'M':
		case 'm':
			font->spacing = BDF_MONOWIDTH;
			break;
		case 'P':
		case 'p':
			font->spacing = BDF_PROPORTIONAL;
			break;
		}
	}
	if (list.size > 0)
		free((char *) list.field);
}

/*
 * Determine if the property is an atom or not.  If it is, then clean it up so
 * the double quotes are removed if they exist.
 */
static int
#ifdef __STDC__
_bdf_is_atom(char *line, unsigned long linelen, char **name, char **value)
#else
_bdf_is_atom(line, linelen, name, value)
char *line;
unsigned long linelen;
char **name, **value;
#endif
{
	int hold;
	char *sp, *ep;
	bdf_property_t *p;

	*name = sp = ep = line;
	while (*ep && *ep != ' ' && *ep != '\t')
		ep++;

	hold = -1;
	if (*ep) {
		hold = *ep;
		*ep = 0;
	}

	p = bdf_get_property(sp);

	/*
	 * Restore the character that was saved before any return can happen.
	 */
	if (hold != -1)
		*ep = hold;

	/*
	 * If the propert exists and is not an atom, just return here.
	 */
	if (p && p->format != BDF_ATOM)
		return 0;

	/*
	 * The property is an atom.  Trim all leading and trailing whitespace and
	 * double quotes for the atom value.
	 */
	sp = ep;
	ep = line + linelen;

	/*
	 * Trim the leading whitespace if it exists.
	 */
	*sp++ = 0;
	while (*sp && (*sp == ' ' || *sp == '\t'))
		sp++;

	/*
	 * Trim the leading double quote if it exists.
	 */
	if (*sp == '"')
		sp++;
	*value = sp;

	/*
	 * Trim the trailing whitespace if it exists.
	 */
	while (ep > sp && (*(ep - 1) == ' ' || *(ep - 1) == '\t'))
		*--ep = 0;

	/*
	 * Trim the trailing double quote if it exists.
	 */
	if (ep > sp && *(ep - 1) == '"')
		*--ep = 0;

	return 1;
}

static void
#ifdef __STDC__
_bdf_add_property(bdf_font_t *font, char *name, char *value)
#else
_bdf_add_property(font, name, value)
bdf_font_t *font;
char *name, *value;
#endif
{
	unsigned long propid;
	hashnode hn;
	int len;
	bdf_property_t *prop, *fp;

	/*
	 * First, check to see if the property already exists in the font.
	 */
	if ((hn = hash_lookup(name, (hashtable *) font->internal)) != 0) {
		/*
		 * The property already exists in the font, so simply replace
		 * the value of the property with the current value.
		 */
		fp = font->props + (unsigned long) hn->data;

		switch (fp->format) {
		case BDF_ATOM:
			/*
			 * Delete the current atom if it exists.
			 */
			if (fp->value.atom != 0)
				free(fp->value.atom);

			if (value == 0)
				len = 1;
			else
				len = strlen(value) + 1;
			if (len > 1) {
				fp->value.atom = (char *) malloc(len);
				(void) memcpy(fp->value.atom, value, len);
			} else
				fp->value.atom = 0;
			break;
		case BDF_INTEGER:
			fp->value.int32 = _bdf_atol(value, 0, 10);
			break;
		case BDF_CARDINAL:
			fp->value.card32 = _bdf_atoul(value, 0, 10);
			break;
		}
		return;
	}

	/*
	 * See if this property type exists yet or not.  If not, create it.
	 */
	hn = hash_lookup(name, &proptbl);
	if (hn == 0) {
		bdf_create_property(name, BDF_ATOM);
		hn = hash_lookup(name, &proptbl);
	}

	/*
	 * Allocate another property if this is overflow.
	 */
	if (font->props_used == font->props_size) {
		if (font->props_size == 0)
			font->props = (bdf_property_t *) malloc(sizeof(bdf_property_t));
		else
			font->props = (bdf_property_t *)
			              realloc((char *) font->props, sizeof(bdf_property_t) *
			                      (font->props_size + 1));
		fp = font->props + font->props_size;
		(void) memset((char *) fp, 0, sizeof(bdf_property_t));
		font->props_size++;
	}

	propid = (unsigned long) hn->data;
	if (propid >= _num_bdf_properties)
		prop = user_props + (propid - _num_bdf_properties);
	else
		prop = _bdf_properties + propid;

	fp = font->props + font->props_used;

	fp->name = prop->name;
	fp->format = prop->format;
	fp->builtin = prop->builtin;

	switch (prop->format) {
	case BDF_ATOM:
		if (value == 0)
			len = 1;
		else
			len = strlen(value) + 1;
		if (len > 1) {
			fp->value.atom = (char *) malloc(len);
			(void) memcpy(fp->value.atom, value, len);
		} else
			fp->value.atom = 0;
		break;
	case BDF_INTEGER:
		fp->value.int32 = _bdf_atol(value, 0, 10);
		break;
	case BDF_CARDINAL:
		fp->value.card32 = _bdf_atoul(value, 0, 10);
		break;
	}

	/*
	 * If the property happens to be a comment, then it doesn't need
	 * to be added to the internal hash table.
	 */
	if (memcmp(name, "COMMENT", 7) != 0)
		/*
		 * Add the property to the font property table.
		 */
		hash_insert(fp->name, (void *) font->props_used,
		            (hashtable *) font->internal);

	font->props_used++;

	/*
	 * Some special cases need to be handled here.  The DEFAULT_CHAR property
	 * needs to be located if it exists in the property list, the FONT_ASCENT
	 * and FONT_DESCENT need to be assigned if they are present, and the
	 * SPACING property should override the default spacing.
	 */
	if (memcmp(name, "DEFAULT_CHAR", 12) == 0)
		font->default_glyph = fp->value.int32;
	else if (memcmp(name, "FONT_ASCENT", 11) == 0)
		font->font_ascent = fp->value.int32;
	else if (memcmp(name, "FONT_DESCENT", 12) == 0)
		font->font_descent = fp->value.int32;
	else if (memcmp(name, "SPACING", 7) == 0) {
		if (fp->value.atom[0] == 'p' || fp->value.atom[0] == 'P')
			font->spacing = BDF_PROPORTIONAL;
		else if (fp->value.atom[0] == 'm' || fp->value.atom[0] == 'M')
			font->spacing = BDF_MONOWIDTH;
		else if (fp->value.atom[0] == 'c' || fp->value.atom[0] == 'C')
			font->spacing = BDF_CHARCELL;
	}
}

/*
 * Actually parse the glyph info and bitmaps.
 */
static int
#ifdef __STDC__
_bdf_parse_glyphs(char *line, unsigned long linelen, unsigned long lineno,
                  void *call_data, void *client_data)
#else
_bdf_parse_glyphs(line, linelen, lineno, call_data, client_data)
char *line;
unsigned long linelen, lineno;
void *call_data, *client_data;
#endif
{
	int c;
	char *s;
	unsigned char *bp;
	unsigned long i, slen, nibbles;
	double ps, rx, dw, sw;
	_bdf_line_func_t *next;
	_bdf_parse_t *p;
	bdf_glyph_t *glyph;
	bdf_font_t *font;
	char nbuf[128];

	next = (_bdf_line_func_t *) call_data;
	p = (_bdf_parse_t *) client_data;

	font = p->font;

	/*
	 * Check for a comment.
	 */
	if (memcmp(line, "COMMENT", 7) == 0) {
		linelen -= 7;
		s = line + 7;
		if (*s != 0) {
			s++;
			linelen--;
		}
		_bdf_add_comment(p->font, s, linelen);
		return 0;
	}

	/*
	 * The very first thing expected is the number of glyphs.
	 */
	if (!(p->flags & _BDF_GLYPHS)) {
		if (memcmp(line, "CHARS", 5) != 0) {
			sprintf(nbuf, ERRMSG1, lineno, "CHARS");
			_bdf_add_acmsg(p->font, nbuf, strlen(nbuf));
			return BDF_MISSING_CHARS;
		}
		_bdf_split(" +", line, linelen, &p->list);
		p->cnt = font->glyphs_size = _bdf_atoul(p->list.field[1], 0, 10);

		/*
		 * Make sure the number of glyphs is non-zero.
		 */
		if (p->cnt == 0)
			font->glyphs_size = 64;

		font->glyphs = (bdf_glyph_t *) malloc(sizeof(bdf_glyph_t) *
		                                      font->glyphs_size);

		/*
		 * Set up the callback to indicate the glyph loading is about to
		 * begin.
		 */
		if (p->callback != 0) {
			p->cb.reason = BDF_LOAD_START;
			p->cb.total = p->cnt;
			p->cb.current = 0;
			(*p->callback)(&p->cb, p->client_data);
		}
		p->flags |= _BDF_GLYPHS;
		return 0;
	}

	/*
	 * Check for the ENDFONT field.
	 */
	if (memcmp(line, "ENDFONT", 7) == 0) {
		/*
		 * Sort the glyphs by encoding.
		 */
		qsort((char *) font->glyphs, font->glyphs_used, sizeof(bdf_glyph_t),
		      by_encoding);

		p->flags &= ~_BDF_START;
		return 0;
	}

	/*
	 * Check for the ENDCHAR field.
	 */
	if (memcmp(line, "ENDCHAR", 7) == 0) {
		/*
		 * Set up and call the callback if it was passed.
		 */
		if (p->callback != 0) {
			p->cb.reason = BDF_LOADING;
			p->cb.total = font->glyphs_size;
			p->cb.current = font->glyphs_used;
			(*p->callback)(&p->cb, p->client_data);
		}
		p->glyph_enc = 0;
		p->flags &= ~_BDF_GLYPH_BITS;
		return 0;
	}

	/*
	 * Check to see if a glyph is being scanned but should be ignored
	 * because it is an unencoded glyph.
	 */
	if ((p->flags & _BDF_GLYPH) &&
	        p->glyph_enc == -1 && p->opts->keep_unencoded == 0)
		return 0;

	/*
	 * Check for the STARTCHAR field.
	 */
	if (memcmp(line, "STARTCHAR", 9) == 0) {
		/*
		 * Set the character name in the parse info first until the
		 * encoding can be checked for an unencoded character.
		 */
		if (p->glyph_name != 0)
			free(p->glyph_name);
		_bdf_split(" +", line, linelen, &p->list);
		_bdf_shift(1, &p->list);
		s = _bdf_join(' ', &slen, &p->list);
		p->glyph_name = (char *) malloc(slen + 1);
		(void) memcpy(p->glyph_name, s, slen + 1);
		p->flags |= _BDF_GLYPH;
		return 0;
	}

	/*
	 * Check for the ENCODING field.
	 */
	if (memcmp(line, "ENCODING", 8) == 0) {
		if (!(p->flags & _BDF_GLYPH)) {
			/*
			 * Missing STARTCHAR field.
			 */
			sprintf(nbuf, ERRMSG1, lineno, "STARTCHAR");
			_bdf_add_acmsg(font, nbuf, strlen(nbuf));
			return BDF_MISSING_STARTCHAR;
		}
		_bdf_split(" +", line, linelen, &p->list);
		p->glyph_enc = _bdf_atol(p->list.field[1], 0, 10);

		/*
		 * Check to see if this encoding has already been encountered.  If it
		 * has then change it to unencoded so it gets added if indicated.
		 */
		if (p->glyph_enc >= 0) {
			if (_bdf_glyph_modified(p->have, p->glyph_enc)) {
				/*
				 * Add a message saying a glyph has been moved to the
				 * unencoded area.
				 */
				sprintf(nbuf, ACMSG12, p->glyph_enc, p->glyph_name);
				_bdf_add_acmsg(font, nbuf, strlen(nbuf));
				p->glyph_enc = -1;
				font->modified = 1;
			} else
				_bdf_set_glyph_modified(p->have, p->glyph_enc);
		}

		if (p->glyph_enc >= 0) {
			/*
			 * Make sure there are enough glyphs allocated in case the
			 * number of characters happen to be wrong.
			 */
			if (font->glyphs_used == font->glyphs_size) {
				font->glyphs = (bdf_glyph_t *)
				               realloc((char *) font->glyphs,
				                       sizeof(bdf_glyph_t) * (font->glyphs_size + 64));
				(void) memset((char *)(font->glyphs + font->glyphs_size),
				              0, sizeof(bdf_glyph_t) << 6);
				font->glyphs_size += 64;
			}

			glyph = font->glyphs + font->glyphs_used++;
			glyph->name = p->glyph_name;
			glyph->encoding = p->glyph_enc;

			/*
			 * Reset the initial glyph info.
			 */
			p->glyph_name = 0;
		} else {
			/*
			 * Unencoded glyph.  Check to see if it should be added or not.
			 */
			if (p->opts->keep_unencoded != 0) {
				/*
				 * Allocate the next unencoded glyph.
				 */
				if (font->unencoded_used == font->unencoded_size) {
					if (font->unencoded_size == 0)
						font->unencoded = (bdf_glyph_t *)
						                  malloc(sizeof(bdf_glyph_t) << 2);
					else
						font->unencoded = (bdf_glyph_t *)
						                  realloc((char *) font->unencoded,
						                          sizeof(bdf_glyph_t) *
						                          (font->unencoded_size + 4));
					font->unencoded_size += 4;
				}

				glyph = font->unencoded + font->unencoded_used;
				glyph->name = p->glyph_name;
				glyph->encoding = font->unencoded_used++;
			} else
				/*
				 * Free up the glyph name if the unencoded shouldn't be
				 * kept.
				 */
				free(p->glyph_name);

			p->glyph_name = 0;
		}

		/*
		 * Clear the flags that might be added when width and height are
		 * checked for consistency.
		 */
		p->flags &= ~(_BDF_GLYPH_WIDTH_CHECK | _BDF_GLYPH_HEIGHT_CHECK);

		p->flags |= _BDF_ENCODING;
		return 0;
	}

	/*
	 * Point at the glyph being constructed.
	 */
	if (p->glyph_enc == -1)
		glyph = font->unencoded + (font->unencoded_used - 1);
	else
		glyph = font->glyphs + (font->glyphs_used - 1);

	/*
	 * Check to see if a bitmap is being constructed.
	 */
	if (p->flags & _BDF_BITMAP) {
		/*
		 * If there are more rows than are specified in the glyph metrics,
		 * ignore the remaining lines.
		 */
		if (p->row >= glyph->bbx.height) {
			if (!(p->flags & _BDF_GLYPH_HEIGHT_CHECK)) {
				sprintf(nbuf, ACMSG13, glyph->encoding);
				_bdf_add_acmsg(font, nbuf, strlen(nbuf));
				p->flags |= _BDF_GLYPH_HEIGHT_CHECK;
				font->modified = 1;
			}
			return 0;
		}

		/*
		 * Only collect the number of nibbles indicated by the glyph metrics.
		 * If there are more columns, they are simply ignored.
		 */
		nibbles = p->bpr << 1;
		bp = glyph->bitmap + (p->row * p->bpr);
		for (i = 0, *bp = 0; i < nibbles; i++) {
			c = line[i];
			*bp = (*bp << 4) + a2i[c];
			if (i + 1 < nibbles && (i & 1))
				*++bp = 0;
		}

		/*
		 * If any line has extra columns, indicate they have been removed.
		 */
		if ((line[nibbles] == '0' || a2i[(int) line[nibbles]] != 0) &&
		        !(p->flags & _BDF_GLYPH_WIDTH_CHECK)) {
			sprintf(nbuf, ACMSG14, glyph->encoding);
			_bdf_add_acmsg(font, nbuf, strlen(nbuf));
			p->flags |= _BDF_GLYPH_WIDTH_CHECK;
			font->modified = 1;
		}

		p->row++;
		return 0;
	}

	/*
	 * Expect the SWIDTH (scalable width) field next.
	 */
	if (memcmp(line, "SWIDTH", 6) == 0) {
		if (!(p->flags & _BDF_ENCODING)) {
			/*
			 * Missing ENCODING field.
			 */
			sprintf(nbuf, ERRMSG1, lineno, "ENCODING");
			_bdf_add_acmsg(font, nbuf, strlen(nbuf));
			return BDF_MISSING_ENCODING;
		}
		_bdf_split(" +", line, linelen, &p->list);
		glyph->swidth = _bdf_atoul(p->list.field[1], 0, 10);
		p->flags |= _BDF_SWIDTH;
		return 0;
	}

	/*
	 * Expect the DWIDTH (scalable width) field next.
	 */
	if (memcmp(line, "DWIDTH", 6) == 0) {
		_bdf_split(" +", line, linelen, &p->list);
		glyph->dwidth = _bdf_atoul(p->list.field[1], 0, 10);

		if (!(p->flags & _BDF_SWIDTH)) {
			/*
			 * Missing SWIDTH field.  Add an auto correction message and set
			 * the scalable width from the device width.
			 */
			sprintf(nbuf, ACMSG9, lineno);
			_bdf_add_acmsg(font, nbuf, strlen(nbuf));
			ps = (double) font->point_size;
			rx = (double) font->resolution_x;
			dw = (double) glyph->dwidth;
			glyph->swidth = (unsigned short)((dw * 72000.0) / (ps * rx));
		}

		p->flags |= _BDF_DWIDTH;
		return 0;
	}

	/*
	 * Expect the BBX field next.
	 */
	if (memcmp(line, "BBX", 3) == 0) {
		_bdf_split(" +", line, linelen, &p->list);
		glyph->bbx.width = _bdf_atos(p->list.field[1], 0, 10);
		glyph->bbx.height = _bdf_atos(p->list.field[2], 0, 10);
		glyph->bbx.x_offset = _bdf_atos(p->list.field[3], 0, 10);
		glyph->bbx.y_offset = _bdf_atos(p->list.field[4], 0, 10);

		/*
		 * Generate the ascent and descent of the character.
		 */
		glyph->bbx.ascent = glyph->bbx.height + glyph->bbx.y_offset;
		glyph->bbx.descent = -glyph->bbx.y_offset;

		/*
		 * Determine the overall font bounding box as the characters are
		 * loaded so corrections can be done later if indicated.
		 */
		p->maxas = MAX(glyph->bbx.ascent, p->maxas);
		p->maxds = MAX(glyph->bbx.descent, p->maxds);
		p->rbearing = glyph->bbx.width + glyph->bbx.x_offset;
		p->maxrb = MAX(p->rbearing, p->maxrb);
		p->minlb = MIN(glyph->bbx.x_offset, p->minlb);
		p->maxlb = MAX(glyph->bbx.x_offset, p->maxlb);

		if (!(p->flags & _BDF_DWIDTH)) {
			/*
			 * Missing DWIDTH field.  Add an auto correction message and set
			 * the device width to the glyph width.
			 */
			sprintf(nbuf, ACMSG10, lineno);
			_bdf_add_acmsg(font, nbuf, strlen(nbuf));
			glyph->dwidth = glyph->bbx.width;
		}

		/*
		 * If the BDF_CORRECT_METRICS flag is set, then adjust the SWIDTH
		 * value if necessary.
		 */
		if (p->opts->correct_metrics != 0) {
			/*
			 * Determine the point size of the glyph.
			 */
			ps = (double) font->point_size;
			rx = (double) font->resolution_x;
			dw = (double) glyph->dwidth;
			sw = (unsigned short)((dw * 72000.0) / (ps * rx));

			if (sw != glyph->swidth) {
				glyph->swidth = (unsigned short) sw;
				if (p->glyph_enc == -1)
					_bdf_set_glyph_modified(font->umod,
					                        font->unencoded_used - 1);
				else
					_bdf_set_glyph_modified(font->nmod, glyph->encoding);
				p->flags |= _BDF_SWIDTH_ADJ;
				font->modified = 1;
			}
		}
		p->flags |= _BDF_BBX;
		return 0;
	}

	/*
	 * And finally, gather up the bitmap.
	 */
	if (memcmp(line, "BITMAP", 6) == 0) {
		if (!(p->flags & _BDF_BBX)) {
			/*
			 * Missing BBX field.
			 */
			sprintf(nbuf, ERRMSG1, lineno, "BBX");
			_bdf_add_acmsg(font, nbuf, strlen(nbuf));
			return BDF_MISSING_BBX;
		}
		/*
		 * Allocate enough space for the bitmap.
		 */
		p->bpr = ((glyph->bbx.width * p->font->bpp) + 7) >> 3;
		glyph->bytes = p->bpr * glyph->bbx.height;
		glyph->bitmap = (unsigned char *) malloc(glyph->bytes);
		p->row = 0;
		p->flags |= _BDF_BITMAP;
		return 0;
	}

	return BDF_INVALID_LINE;
}

/*
 * Load the font properties.
 */
static int
#ifdef __STDC__
_bdf_parse_properties(char *line, unsigned long linelen, unsigned long lineno,
                      void *call_data, void *client_data)
#else
_bdf_parse_properties(line, linelen, lineno, call_data, client_data)
char *line;
unsigned long linelen, lineno;
void *call_data, *client_data;
#endif
{
	unsigned long vlen;
	_bdf_line_func_t *next;
	_bdf_parse_t *p;
	char *name, *value, nbuf[128];

	next = (_bdf_line_func_t *) call_data;
	p = (_bdf_parse_t *) client_data;

	/*
	 * Check for the end of the properties.
	 */
	if (memcmp(line, "ENDPROPERTIES", 13) == 0) {
		/*
		 * If the FONT_ASCENT or FONT_DESCENT properties have not been
		 * encountered yet, then make sure they are added as properties and
		 * make sure they are set from the font bounding box info.
		 *
		 * This is *always* done regardless of the options, because X11
		 * requires these two fields to compile fonts.
		 */
		if (bdf_get_font_property(p->font, "FONT_ASCENT") == 0) {
			p->font->font_ascent = p->font->bbx.ascent;
			sprintf(nbuf, "%hd", p->font->bbx.ascent);
			_bdf_add_property(p->font, "FONT_ASCENT", nbuf);
			sprintf(nbuf, ACMSG1, p->font->bbx.ascent);
			_bdf_add_acmsg(p->font, nbuf, strlen(nbuf));
			p->font->modified = 1;
		}
		if (bdf_get_font_property(p->font, "FONT_DESCENT") == 0) {
			p->font->font_descent = p->font->bbx.descent;
			sprintf(nbuf, "%hd", p->font->bbx.descent);
			_bdf_add_property(p->font, "FONT_DESCENT", nbuf);
			sprintf(nbuf, ACMSG2, p->font->bbx.descent);
			_bdf_add_acmsg(p->font, nbuf, strlen(nbuf));
			p->font->modified = 1;
		}
		p->flags &= ~_BDF_PROPS;
		*next = _bdf_parse_glyphs;
		return 0;
	}

	/*
	 * Ignore the _XFREE86_GLYPH_RANGES properties.
	 */
	if (memcmp(line, "_XFREE86_GLYPH_RANGES", 21) == 0)
		return 0;

	/*
	 * Handle COMMENT fields and properties in a special way to preserve
	 * the spacing.
	 */
	if (memcmp(line, "COMMENT", 7) == 0) {
		name = value = line;
		value += 7;
		if (*value)
			*value++ = 0;
		_bdf_add_property(p->font, name, value);
	} else if (_bdf_is_atom(line, linelen, &name, &value))
		_bdf_add_property(p->font, name, value);
	else {
		_bdf_split(" +", line, linelen, &p->list);
		name = p->list.field[0];
		_bdf_shift(1, &p->list);
		value = _bdf_join(' ', &vlen, &p->list);
		_bdf_add_property(p->font, name, value);
	}

	return 0;
}

/*
 * Load the font header.
 */
static int
#ifdef __STDC__
_bdf_parse_start(char *line, unsigned long linelen, unsigned long lineno,
                 void *call_data, void *client_data)
#else
_bdf_parse_start(line, linelen, lineno, call_data, client_data)
char *line;
unsigned long linelen, lineno;
void *call_data, *client_data;
#endif
{
	unsigned long slen;
	_bdf_line_func_t *next;
	_bdf_parse_t *p;
	bdf_font_t *font;
	char *s, nbuf[128];

	next = (_bdf_line_func_t *) call_data;
	p = (_bdf_parse_t *) client_data;

	/*
	 * Check for a comment.  This is done to handle those fonts that have
	 * comments before the STARTFONT line for some reason.
	 */
	if (memcmp(line, "COMMENT", 7) == 0) {
		if (p->opts->keep_comments != 0 && p->font != 0) {
			linelen -= 7;
			s = line + 7;
			if (*s != 0) {
				s++;
				linelen--;
			}
			_bdf_add_comment(p->font, s, linelen);
		}
		return 0;
	}

	if (!(p->flags & _BDF_START)) {
		if (memcmp(line, "STARTFONT", 9) != 0)
			/*
			 * No STARTFONT field is a good indication of a problem.
			 */
			return BDF_MISSING_START;
		p->flags = _BDF_START;
		p->font = font = (bdf_font_t *) calloc(1, sizeof(bdf_font_t));
		p->font->internal = (void *) malloc(sizeof(hashtable));
		hash_init((hashtable *) p->font->internal);
		p->font->spacing = p->opts->font_spacing;
		p->font->default_glyph = -1;
		return 0;
	}

	/*
	 * Check for the start of the properties.
	 */
	if (memcmp(line, "STARTPROPERTIES", 15) == 0) {
		_bdf_split(" +", line, linelen, &p->list);
		p->cnt = p->font->props_size = _bdf_atoul(p->list.field[1], 0, 10);
		p->font->props = (bdf_property_t *)
		                 malloc(sizeof(bdf_property_t) * p->cnt);
		p->flags |= _BDF_PROPS;
		*next = _bdf_parse_properties;
		return 0;
	}

	/*
	 * Check for the FONTBOUNDINGBOX field.
	 */
	if (memcmp(line, "FONTBOUNDINGBOX", 15) == 0) {
		if (!(p->flags & _BDF_SIZE)) {
			/*
			 * Missing the SIZE field.
			 */
			sprintf(nbuf, ERRMSG1, lineno, "SIZE");
			_bdf_add_acmsg(p->font, nbuf, strlen(nbuf));
			return BDF_MISSING_SIZE;
		}
		_bdf_split(" +", line, linelen, &p->list);
		p->font->bbx.width = _bdf_atos(p->list.field[1], 0, 10);
		p->font->bbx.height = _bdf_atos(p->list.field[2], 0, 10);
		p->font->bbx.x_offset = _bdf_atos(p->list.field[3], 0, 10);
		p->font->bbx.y_offset = _bdf_atos(p->list.field[4], 0, 10);
		p->font->bbx.ascent = p->font->bbx.height + p->font->bbx.y_offset;
		p->font->bbx.descent = -p->font->bbx.y_offset;
		p->flags |= _BDF_FONT_BBX;
		return 0;
	}

	/*
	 * The next thing to check for is the FONT field.
	 */
	if (memcmp(line, "FONT", 4) == 0) {
		_bdf_split(" +", line, linelen, &p->list);
		_bdf_shift(1, &p->list);
		s = _bdf_join(' ', &slen, &p->list);
		p->font->name = (char *) malloc(slen + 1);
		(void) memcpy(p->font->name, s, slen + 1);
		/*
		 * If the font name is an XLFD name, set the spacing to the one in the
		 * font name.  If there is no spacing fall back on the default.
		 */
		_bdf_set_default_spacing(p->font, p->opts);
		p->flags |= _BDF_FONT_NAME;
		return 0;
	}

	/*
	 * Check for the SIZE field.
	 */
	if (memcmp(line, "SIZE", 4) == 0) {
		if (!(p->flags & _BDF_FONT_NAME)) {
			/*
			 * Missing the FONT field.
			 */
			sprintf(nbuf, ERRMSG1, lineno, "FONT");
			_bdf_add_acmsg(p->font, nbuf, strlen(nbuf));
			return BDF_MISSING_FONTNAME;
		}
		_bdf_split(" +", line, linelen, &p->list);
		p->font->point_size = _bdf_atoul(p->list.field[1], 0, 10);
		p->font->resolution_x = _bdf_atoul(p->list.field[2], 0, 10);
		p->font->resolution_y = _bdf_atoul(p->list.field[3], 0, 10);

		/*
		 * Check for the bits per pixel field.
		 */
		if (p->list.used == 5) {
			p->font->bpp = _bdf_atos(p->list.field[4], 0, 10);
			if (p->font->bpp > 1 && (p->font->bpp & 1)) {
				/*
				 * Move up to the next bits per pixel value if an odd number
				 * is encountered.
				 */
				p->font->bpp++;
				if (p->font->bpp <= 4) {
					sprintf(nbuf, ACMSG11, p->font->bpp);
					_bdf_add_acmsg(p->font, nbuf, strlen(nbuf));
				}
			}
			if (p->font->bpp > 4) {
				sprintf(nbuf, ACMSG11, p->font->bpp);
				_bdf_add_acmsg(p->font, nbuf, strlen(nbuf));
				p->font->bpp = 4;
			}
		} else
			p->font->bpp = 1;

		p->flags |= _BDF_SIZE;
		return 0;
	}

	return BDF_INVALID_LINE;
}

/**************************************************************************
 *
 * API.
 *
 **************************************************************************/

void
#ifdef __STDC__
bdf_setup(void)
#else
bdf_setup()
#endif
{
	unsigned long i;
	bdf_property_t *prop;

	hash_init(&proptbl);
	for (i = 0, prop = _bdf_properties; i < _num_bdf_properties; i++, prop++)
		hash_insert(prop->name, (void *) i, &proptbl);
}

void
#ifdef __STDC__
bdf_cleanup(void)
#else
bdf_cleanup()
#endif
{
	unsigned long i;
	bdf_property_t *prop;

	hash_free(&proptbl);

	/*
	 * Free up the user defined properties.
	 */
	for (prop = user_props, i = 0; i < nuser_props; i++, prop++) {
		free(prop->name);
		if (prop->format == BDF_ATOM && prop->value.atom != 0)
			free(prop->value.atom);
	}
	if (nuser_props > 0)
		free((char *) user_props);

	_bdf_glyph_name_cleanup();
}

bdf_font_t *
#ifdef __STDC__
bdf_load_font(FILE *in, bdf_options_t *opts, bdf_callback_t callback,
              void *data)
#else
bdf_load_font(in, opts, callback, data)
FILE *in;
bdf_options_t *opts;
bdf_callback_t callback;
void *data;
#endif
{
	int n;
	unsigned long lineno;
	char msgbuf[128];
	_bdf_parse_t p;

	(void) memset((char *) &p, 0, sizeof(_bdf_parse_t));
	p.opts = (opts != 0) ? opts : &_bdf_opts;
	p.minlb = 32767;
	p.callback = callback;
	p.client_data = data;
	n = _bdf_readlines(fileno(in), _bdf_parse_start, (void *) & p, &lineno);

	if (p.font != 0) {
		/*
		 * If the font is not proportional, set the fonts monowidth
		 * field to the width of the font bounding box.
		 */
		if (p.font->spacing != BDF_PROPORTIONAL)
			p.font->monowidth = p.font->bbx.width;

		/*
		 * If the number of glyphs loaded is not that of the original count,
		 * indicate the difference.
		 */
		if (p.cnt != p.font->glyphs_used + p.font->unencoded_used) {
			sprintf(msgbuf, ACMSG15, p.cnt,
			        p.font->glyphs_used + p.font->unencoded_used);
			_bdf_add_acmsg(p.font, msgbuf, strlen(msgbuf));
			p.font->modified = 1;
		}

		/*
		 * Once the font has been loaded, adjust the overall font metrics if
		 * necessary.
		 */
		if (p.opts->correct_metrics != 0 &&
		        (p.font->glyphs_used > 0 || p.font->unencoded_used > 0)) {
			if (p.maxrb - p.minlb != p.font->bbx.width) {
				sprintf(msgbuf, ACMSG3, p.font->bbx.width, p.maxrb - p.minlb);
				_bdf_add_acmsg(p.font, msgbuf, strlen(msgbuf));
				p.font->bbx.width = p.maxrb - p.minlb;
				p.font->modified = 1;
			}
			if (p.font->bbx.x_offset != p.minlb) {
				sprintf(msgbuf, ACMSG4, p.font->bbx.x_offset, p.minlb);
				_bdf_add_acmsg(p.font, msgbuf, strlen(msgbuf));
				p.font->bbx.x_offset = p.minlb;
				p.font->modified = 1;
			}
			if (p.font->bbx.ascent != p.maxas) {
				sprintf(msgbuf, ACMSG5, p.font->bbx.ascent, p.maxas);
				_bdf_add_acmsg(p.font, msgbuf, strlen(msgbuf));
				p.font->bbx.ascent = p.maxas;
				p.font->modified = 1;
			}
			if (p.font->bbx.descent != p.maxds) {
				sprintf(msgbuf, ACMSG6, p.font->bbx.descent, p.maxds);
				_bdf_add_acmsg(p.font, msgbuf, strlen(msgbuf));
				p.font->bbx.descent = p.maxds;
				p.font->bbx.y_offset = -p.maxds;
				p.font->modified = 1;
			}
			if (p.maxas + p.maxds != p.font->bbx.height) {
				sprintf(msgbuf, ACMSG7, p.font->bbx.height, p.maxas + p.maxds);
				_bdf_add_acmsg(p.font, msgbuf, strlen(msgbuf));
			}
			p.font->bbx.height = p.maxas + p.maxds;

			if (p.flags & _BDF_SWIDTH_ADJ)
				_bdf_add_acmsg(p.font, ACMSG8, strlen(ACMSG8));
		}
	}

	/*
	 * Last, if an error happened during loading, handle the messages.
	 */
	if (n < 0 && callback != 0) {
		/*
		 * An error was returned.  Alert the client.
		 */
		p.cb.reason = BDF_ERROR;
		p.cb.errlineno = lineno;
		(*callback)(&p.cb, data);
	} else if (p.flags & _BDF_START) {
		if (p.font != 0) {
			/*
			 * The ENDFONT field was never reached or did not exist.
			 */
			if (!(p.flags & _BDF_GLYPHS))
				/*
				 * Error happened while parsing header.
				 */
				sprintf(msgbuf, ERRMSG2, lineno);
			else
				/*
				 * Error happened when parsing glyphs.
				 */
				sprintf(msgbuf, ERRMSG3, lineno);

			_bdf_add_acmsg(p.font, msgbuf, strlen(msgbuf));
		}

		if (callback != 0) {
			p.cb.reason = BDF_ERROR;
			p.cb.errlineno = lineno;
			(*callback)(&p.cb, data);
		}
	} else if (callback != 0) {
		/*
		 * This forces the progress bar to always finish.
		 */
		p.cb.current = p.cb.total;
		(*p.callback)(&p.cb, p.client_data);
	}

	/*
	 * Free up the list used during the parsing.
	 */
	if (p.list.size > 0)
		free((char *) p.list.field);

	if (p.font != 0) {
		/*
		 * Make sure the comments are NULL terminated if they exist.
		 */
		if (p.font->comments_len > 0) {
			p.font->comments = (char *) realloc(p.font->comments,
			                                    p.font->comments_len + 1);
			p.font->comments[p.font->comments_len] = 0;
		}

		/*
		 * Make sure the auto-correct messages are NULL terminated if they
		 * exist.
		 */
		if (p.font->acmsgs_len > 0) {
			p.font->acmsgs = (char *) realloc(p.font->acmsgs,
			                                  p.font->acmsgs_len + 1);
			p.font->acmsgs[p.font->acmsgs_len] = 0;
		}
	}

	return p.font;
}

#ifdef HAVE_HBF
static int
#ifdef __STDC__
_bdf_parse_hbf_header(char *line, unsigned long linelen, unsigned long lineno,
                      void *call_data, void *client_data)
#else
_bdf_parse_hbf_header(line, linelen, lineno, call_data, client_data)
char *line;
unsigned long linelen, lineno;
void *call_data, *client_data;
#endif
{
	unsigned long vlen;
	char *name, *value;
	_bdf_parse_t *p;
	_bdf_line_func_t *next;
	char nbuf[24];

	next = (_bdf_line_func_t *) call_data;
	p = (_bdf_parse_t *) client_data;

	/*
	 * Check for comments.
	 */
	if (memcmp(line, "COMMENT", 7) == 0) {
		if (p->opts->keep_comments != 0 && p->font != 0) {
			name = line;
			value = name + 7;
			vlen = linelen - 7;
			if (*value) {
				*value++ = 0;
				vlen--;
			}
			/*
			 * If the properties are being parsed, add the comment as a
			 * property.  Otherwise, simply add the comment in the normal
			 * fashion.
			 */
			if (p->flags & _BDF_PROPS)
				_bdf_add_property(p->font, name, value);
			else
				_bdf_add_comment(p->font, value, vlen);
		}
		return 0;
	}

	if (!(p->flags & _BDF_START)) {
		if (memcmp(line, "HBF_START_FONT", 14) != 0)
			return -1;
		p->flags = _BDF_START;
		p->font = (bdf_font_t *) calloc(1, sizeof(bdf_font_t));
		/*
		 * HBF fonts are always assumed to be 1 bit per pixel.
		 */
		p->font->bpp = 1;
		p->font->internal = (void *) malloc(sizeof(hashtable));
		hash_init((hashtable *) p->font->internal);
		p->font->hbf = 1;
		p->font->spacing = p->opts->font_spacing;
		p->font->default_glyph = -1;
		return 0;
	}

	/*
	 * Check for the HBF_END_FONT field.
	 */
	if (memcmp(line, "HBF_END_FONT", 12) == 0)
		/*
		 * Need to perform some checks here to see whether some fields are
		 * missing or not.
		 */
		return 0;

	/*
	 * Check for HBF keywords which will be added as comments.  These should
	 * never occur in the properties list.  Assume they won't.
	 */
	if (memcmp(line, "HBF_", 4) == 0) {
		if (p->opts->keep_comments != 0)
			_bdf_add_comment(p->font, line, linelen);
		return 0;
	}

	if (!(p->flags & _BDF_PROPS)) {
		/*
		 * Check for the start of the properties.
		 */
		if (memcmp(line, "STARTPROPERTIES", 15) == 0) {
			_bdf_split(" +", line, linelen, &p->list);
			p->cnt = p->font->props_size = _bdf_atoul(p->list.field[1], 0, 10);
			p->font->props = (bdf_property_t *)
			                 malloc(sizeof(bdf_property_t) * p->cnt);
			p->flags |= _BDF_PROPS;
			return 0;
		}

		/*
		 * Check for the CHARS field.
		 */
		if (memcmp(line, "CHARS", 5) == 0) {
			_bdf_split(" +", line, linelen, &p->list);
			p->cnt = p->font->glyphs_size =
			             _bdf_atoul(p->list.field[1], 0, 10);
			p->font->glyphs = (bdf_glyph_t *)
			                  malloc(sizeof(bdf_glyph_t) * p->cnt);
			return 0;
		}

		/*
		 * Check for the FONTBOUNDINGBOX field.
		 */
		if (memcmp(line, "FONTBOUNDINGBOX", 15) == 0) {
			if (!(p->flags & (_BDF_START | _BDF_FONT_NAME | _BDF_SIZE)))
				return -1;
			_bdf_split(" +", line, linelen, &p->list);
			p->font->bbx.width = _bdf_atos(p->list.field[1], 0, 10);
			p->font->bbx.height = _bdf_atos(p->list.field[2], 0, 10);
			p->font->bbx.x_offset = _bdf_atos(p->list.field[3], 0, 10);
			p->font->bbx.y_offset = _bdf_atos(p->list.field[4], 0, 10);
			p->font->bbx.ascent = p->font->bbx.height + p->font->bbx.y_offset;
			p->font->bbx.descent = -p->font->bbx.y_offset;
			p->flags |= _BDF_FONT_BBX;
			return 0;
		}

		/*
		 * The next thing to check for is the FONT field.
		 */
		if (memcmp(line, "FONT", 4) == 0) {
			if (!(p->flags & _BDF_START))
				return -1;
			_bdf_split(" +", line, linelen, &p->list);
			_bdf_shift(1, &p->list);
			value = _bdf_join(' ', &vlen, &p->list);
			p->font->name = (char *) malloc(vlen + 1);
			(void) memcpy(p->font->name, value, vlen + 1);
			/*
			 * If the font name is an XLFD name, set the spacing to the one in
			 * the font name.  If there is no spacing fall back on the
			 * default.
			 */
			_bdf_set_default_spacing(p->font, p->opts);
			p->flags |= _BDF_FONT_NAME;
			return 0;
		}

		/*
		 * Check for the SIZE field.
		 */
		if (memcmp(line, "SIZE", 4) == 0) {
			if (!(p->flags & (_BDF_START | _BDF_FONT_NAME)))
				return -1;
			_bdf_split(" +", line, linelen, &p->list);
			p->font->point_size = _bdf_atoul(p->list.field[1], 0, 10);
			p->font->resolution_x = _bdf_atoul(p->list.field[2], 0, 10);
			p->font->resolution_y = _bdf_atoul(p->list.field[3], 0, 10);
			p->flags |= _BDF_SIZE;
			return 0;
		}
	} else {
		/*
		 * Check for the end of the properties.
		 */
		if (memcmp(line, "ENDPROPERTIES", 13) == 0) {
			/*
			 * If the FONT_ASCENT or FONT_DESCENT properties have not been
			 * encountered yet, then make sure they are added as properties and
			 * make sure they are set from the font bounding box info.
			 *
			 * This is *always* done regardless of the options, because X11
			 * requires these two fields to compile fonts.
			 */
			if (bdf_get_font_property(p->font, "FONT_ASCENT") == 0) {
				p->font->font_ascent = p->font->bbx.ascent;
				sprintf(nbuf, "%hd", p->font->bbx.ascent);
				_bdf_add_property(p->font, "FONT_ASCENT", nbuf);
				sprintf(nbuf, ACMSG1, p->font->bbx.ascent);
				_bdf_add_acmsg(p->font, nbuf, strlen(nbuf));
				p->font->modified = 1;
			}
			if (bdf_get_font_property(p->font, "FONT_DESCENT") == 0) {
				p->font->font_descent = p->font->bbx.descent;
				sprintf(nbuf, "%hd", p->font->bbx.descent);
				_bdf_add_property(p->font, "FONT_DESCENT", nbuf);
				sprintf(nbuf, ACMSG2, p->font->bbx.descent);
				_bdf_add_acmsg(p->font, nbuf, strlen(nbuf));
				p->font->modified = 1;
			}
			p->flags &= ~_BDF_PROPS;
			return 0;
		}

		/*
		 * Handle the next thing in the usual property fashion.
		 */
		if (_bdf_is_atom(line, linelen, &name, &value))
			_bdf_add_property(p->font, name, value);
		else {
			_bdf_split(" +", line, linelen, &p->list);
			name = p->list.field[0];
			_bdf_shift(1, &p->list);
			value = _bdf_join(' ', &vlen, &p->list);
			_bdf_add_property(p->font, name, value);
		}
		return 0;
	}

	/*
	 * Anything else is an error.
	 */
	return -1;
}

#ifdef __STDC__
#define CONST const
#else
#define CONST
#endif

static void
#ifdef __STDC__
_bdf_add_hbf_glyph(HBF *hbf, unsigned int code, void *callback_data)
#else
_bdf_add_hbf_glyph(hbf, code, callback_data)
HBF *hbf;
unsigned int code;
void *callback_data;
#endif
{
	CONST unsigned char *bmap;
	unsigned long n;
	bdf_glyph_t *gp;
	bdf_font_t *font;
	_bdf_parse_t *p;
	HBF_BBOX *fbbx;
	double ps, rx, dw;
	char nbuf[24];

	/*
	 * Attempt to get the bitmap.
	 */
	if ((bmap = hbfGetBitmap(hbf, code)) == 0)
		/*
		 * Need some sort of error handling here.
		 */
		return;

	p = (_bdf_parse_t *) callback_data;

	fbbx = hbfFontBBox(hbf);

	font = p->font;

	/*
	 * Check to make sure there is enough space to hold this glyph.  If not,
	 * allocate 10 more just in case.
	 */
	if (font->glyphs_used == font->glyphs_size) {
		if (font->glyphs_size == 0)
			font->glyphs = (bdf_glyph_t *) malloc(sizeof(bdf_glyph_t) * 16);
		else
			font->glyphs = (bdf_glyph_t *)
			               realloc((char *) font->glyphs,
			                       sizeof(bdf_glyph_t) * (font->glyphs_used + 16));
		gp = font->glyphs + font->glyphs_size;
		(void) memset((char *) gp, 0, sizeof(bdf_glyph_t) * 16);
		font->glyphs_size += 16;
	}

	gp = font->glyphs + font->glyphs_used++;

	/*
	 * Set the glyph name.
	 */
	sprintf(nbuf, "char%d", code);
	n = (unsigned long) strlen(nbuf);
	gp->name = (char *) malloc(n + 1);
	(void) memcpy(gp->name, nbuf, n + 1);

	/*
	 * Set encoding.
	 */
	gp->encoding = (long) code;

	/*
	 * Set the device width.
	 */
	gp->dwidth = (unsigned short) fbbx->hbf_width;

	/*
	 * Set the scalable width.
	 */
	ps = (double) font->point_size;
	rx = (double) font->resolution_x;
	dw = (double) gp->dwidth;
	gp->swidth = (unsigned short)((dw * 72000.0) / (ps * rx));

	/*
	 * Set the glyph bounding box.
	 */
	gp->bbx.width = fbbx->hbf_width;
	gp->bbx.height = fbbx->hbf_height;
	gp->bbx.x_offset = fbbx->hbf_xDisplacement;
	gp->bbx.y_offset = fbbx->hbf_yDisplacement;
	gp->bbx.ascent = gp->bbx.height + gp->bbx.y_offset;
	gp->bbx.descent = -gp->bbx.y_offset;

	/*
	 * Add the bitmap by making a copy.  Assumes the font bbx is OK for
	 * determining the number of bytes needed for the glyph bitmap.
	 */
	gp->bytes = ((gp->bbx.width + 7) >> 3) * gp->bbx.height;
	gp->bitmap = (unsigned char *) malloc(gp->bytes);
	(void) memcpy((char *) gp->bitmap, (char *) bmap, gp->bytes);

	/*
	 * Call the callback if it was provided.
	 */
	if (p->callback != 0) {
		p->cb.reason = BDF_LOADING;
		p->cb.total = font->glyphs_size;
		p->cb.current = font->glyphs_used;
		(*p->callback)(&p->cb, p->client_data);
	}
}

bdf_font_t *
#ifdef __STDC__
bdf_load_hbf_font(char *filename, bdf_options_t *opts, bdf_callback_t callback,
                  void *data)
#else
bdf_load_hbf_font(filename, opts, callback, data)
char *filename;
bdf_options_t *opts;
bdf_callback_t callback;
void *data;
#endif
{
	int n, diff;
	unsigned long lineno;
	FILE *in;
	HBF *hbf;
	bdf_property_t *pp;
	char *name;
	_bdf_parse_t p;

	if ((hbf = hbfOpen(filename)) == 0)
		return 0;

	if ((in = fopen(hbfFileName(hbf), "r")) == 0) {
		hbfClose(hbf);
		return 0;
	}

	/*
	 * Parse the HBF header for properties and other things.
	 */
	(void) memset((char *) &p, 0, sizeof(_bdf_parse_t));
	p.opts = (opts != 0) ? opts : &_bdf_opts;
	p.minlb = 32767;
	p.callback = callback;
	p.client_data = data;

	n = _bdf_readlines(fileno(in), _bdf_parse_hbf_header, (void *) & p,
	                   &lineno);

	fclose(in);

	/*
	 * Determine what spacing the font has so the monowidth field can be set
	 * if necessary.
	 */
	if ((pp = bdf_get_font_property(p.font, "SPACING")) != 0) {
		switch (pp->value.atom[0]) {
		case 'p':
		case 'P':
			p.font->spacing = BDF_PROPORTIONAL;
			break;
		case 'm':
		case 'M':
			p.font->spacing = BDF_MONOWIDTH;
			break;
		case 'c':
		case 'C':
			p.font->spacing = BDF_CHARCELL;
			break;
		}
	}

	/*
	 * Set the monowidth field if necessary.
	 */
	if (p.font->spacing != BDF_PROPORTIONAL)
		p.font->monowidth = p.font->bbx.width;

	/*
	 * Before loading the glyphs, check to see if any glyph structures have
	 * been added.  If not, check the HBF font for the number of characters.
	 * Dynamically increasing glyph storage causes memory fragmentation on
	 * some machines and crashes.  This takes care of the cases where the HBF
	 * file does not provide a "CHARS n" line.
	 */
	if (p.font->glyphs_size < hbfChars(hbf)) {
		if (p.font->glyphs_size == 0)
			p.font->glyphs = (bdf_glyph_t *)
			                 malloc(sizeof(bdf_glyph_t) * hbfChars(hbf));
		else
			p.font->glyphs = (bdf_glyph_t *)
			                 realloc((char *) p.font->glyphs,
			                         sizeof(bdf_glyph_t) * hbfChars(hbf));
		diff = hbfChars(hbf) - p.font->glyphs_size;
		(void) memset((char *)(p.font->glyphs + p.font->glyphs_size), 0,
		              diff);
		p.font->glyphs_size = hbfChars(hbf);
	}

	/*
	 * Call the callback initially to set things up.
	 */
	if (p.callback != 0) {
		p.cb.reason = BDF_LOAD_START;
		p.cb.total = p.font->glyphs_size;
		p.cb.current = 0;
		(*p.callback)(&p.cb, p.client_data);
	}

	/*
	 * Now load the glyphs.
	 */
	hbfForEach(hbf, _bdf_add_hbf_glyph, (void *) &p);

	/*
	 * Close the HBF font.
	 */
	hbfClose(hbf);

	/*
	 * Sort the glyphs by encoding.
	 */
	qsort((char *) p.font->glyphs, p.font->glyphs_used, sizeof(bdf_glyph_t),
	      by_encoding);

	/*
	 * After loading the HBF header, create an XLFD name.  If the XLFD name
	 * cannot be made then preserve the name found in the HBF file.
	 */
	if ((name = bdf_make_xlfd_name(p.font, "HBF", "Unknown")) != 0) {
		if (p.font->name != 0)
			/*
			 * If a name already exists in the font, free it up.
			 */
			free(p.font->name);

		/*
		 * Replace the old name with the XLFD name.
		 */
		p.font->name = name;
	}

	/*
	 * Mark the font as being modified and generate a message that says
	 * something about the font being converted from HBF format.
	 */
	p.font->modified = 1;
	_bdf_add_acmsg(p.font, "Font converted from HBF to BDF.", 31);

	return p.font;
}
#endif /* HAVE_HBF */

/*
 * Crop the glyph bitmap to the minimum rectangle needed to hold the bits that
 * are set.  Adjust the metrics based on the provided bounding box.
 */
static void
#ifdef __STDC__
_bdf_crop_glyph(bdf_font_t *font, bdf_glyph_t *glyph)
#else
_bdf_crop_glyph(font, glyph)
bdf_font_t *font;
bdf_glyph_t *glyph;
#endif
{
	int byte;
	unsigned short x, y, bpr, nbpr, col, colx, si, di;
	unsigned short minx, maxx, miny, maxy;
	unsigned long bytes;
	unsigned char *bmap, *masks;
	bdf_bbx_t nbbx;

	if (glyph == 0)
		return;

	(void) memcpy((char *) &nbbx, (char *) &glyph->bbx, sizeof(bdf_bbx_t));

	bpr = ((glyph->bbx.width * font->bpp) + 7) >> 3;

	maxx = maxy = 0;
	minx = miny = 32767;

	masks = 0;
	switch (font->bpp) {
	case 1:
		masks = onebpp;
		break;
	case 2:
		masks = twobpp;
		break;
	case 4:
		masks = fourbpp;
		break;
	}

	for (y = 0; y < glyph->bbx.height; y++) {
		for (col = x = 0; x < glyph->bbx.width; x++, col += font->bpp) {
			si = (col & 7) / font->bpp;
			if (glyph->bitmap[(y * bpr) + (col >> 3)] & masks[si]) {
				minx = MIN(minx, x);
				maxx = MAX(maxx, x);
				miny = MIN(miny, y);
				maxy = MAX(maxy, y);
			}
		}
	}

	/*
	 * Handle an empty bitmap as a special case.
	 */
	if (minx == 32767) {
		if (glyph->bytes > 0)
			free((char *) glyph->bitmap);
		glyph->bytes = 0;
		(void) memset((char *) &glyph->bbx, 0, sizeof(bdf_bbx_t));
		return;
	}

	/*
	 * Increment the max points so width and height calculations won't go
	 * wrong.
	 */
	maxx++;
	maxy++;

	if (minx > 0)
		nbbx.x_offset += minx;
	if (maxx - minx != nbbx.width)
		nbbx.width = maxx - minx;

	if (miny > 0)
		nbbx.ascent -= miny;
	if (maxy - miny != nbbx.height)
		nbbx.height = maxy - miny;
	nbbx.descent = nbbx.height - nbbx.ascent;
	nbbx.y_offset = -nbbx.descent;

	nbpr = ((nbbx.width * font->bpp) + 7) >> 3;

	/*
	 * If nothing changed, then the glyph is already contained in the
	 * minimum rectangle.
	 */
	if (memcmp((char *) &nbbx, (char *) &glyph->bbx,
	           sizeof(bdf_bbx_t)) == 0 ||
	        (nbpr == bpr && nbbx.height == glyph->bbx.height))
		return;

	/*
	 * The metrics changed, so a new bitmap is needed.
	 */
	bytes = nbpr * nbbx.height;
	bmap = (unsigned char *) malloc(bytes);
	(void) memset((char *) bmap, 0, bytes);

	colx = minx * font->bpp;
	for (y = miny; y < maxy; y++) {
		for (col = x = minx; x < maxx; x++, col += font->bpp) {
			si = (col & 7) / font->bpp;
			byte = glyph->bitmap[(y * bpr) + (col >> 3)] & masks[si];
			if (byte) {
				/*
				 * Position the pixel in the byte if necessary.
				 */
				di = ((col - colx) & 7) / font->bpp;
				if (di < si)
					byte <<= (si - di) * font->bpp;
				else if (di > si)
					byte >>= (di - si) * font->bpp;
				bmap[((y - miny) * nbpr) + ((col - colx) >> 3)] |= byte;
			}
		}
	}

	if (glyph->bytes > 0)
		free((char *) glyph->bitmap);
	glyph->bytes = bytes;
	glyph->bitmap = bmap;

	(void) memcpy((char *) &glyph->bbx, (char *) &nbbx, sizeof(bdf_bbx_t));
}

/*
 * Pad a character-cell font glyph to match the bounds specified in the
 * provided bounding box.
 */
static void
#ifdef __STDC__
_bdf_pad_cell(bdf_font_t *font, bdf_glyph_t *glyph, bdf_glyph_t *cell)
#else
_bdf_pad_cell(font, glyph, cell)
bdf_font_t *font;
bdf_glyph_t *glyph, *cell;
#endif
{
	bdf_bbx_t *bbx;
	unsigned short si, di, sx, byte;
	unsigned short x, y, dx, dy, bx, by, bpr, nbpr;
	unsigned char *bmap, *masks;

	masks = 0;
	switch (font->bpp) {
	case 1:
		masks = onebpp;
		break;
	case 2:
		masks = twobpp;
		break;
	case 4:
		masks = fourbpp;
		break;
	}

	bbx = &font->bbx;

	if (glyph->bbx.width == bbx->width && glyph->bbx.height == bbx->height) {
		/*
		 * The glyph is already positioned in the cell.  Copy the bitmap
		 * and return.
		 */
		(void) memcpy((char *) cell->bitmap, (char *) glyph->bitmap,
		              cell->bytes);
		return;
	}

	/*
	 * Determine the X and Y location of the baseline.
	 */
	bx = MYABS(bbx->x_offset - glyph->bbx.x_offset);
	by = (bbx->ascent + bbx->descent) + bbx->y_offset;

	bpr = ((glyph->bbx.width * font->bpp) + 7) >> 3;
	nbpr = ((bbx->width * font->bpp) + 7) >> 3;

	/*
	 * Set various cell values and clear the cell bitmap.
	 */
	bmap = cell->bitmap;
	(void) memset((char *) bmap, 0, cell->bytes);

	for (dy = by - glyph->bbx.ascent, y = 0; y < glyph->bbx.height;
	        y++, dy++) {
		for (dx = bx * font->bpp, sx = x = 0; x < glyph->bbx.width;
		        x++, dx += font->bpp, sx += font->bpp) {
			si = (sx & 7) / font->bpp;
			byte = glyph->bitmap[(y * bpr) + (sx >> 3)] & masks[si];
			if (byte) {
				di = (dx & 7) / font->bpp;
				if (di < si)
					byte <<= (si - di) * font->bpp;
				else if (di > si)
					byte >>= (di - si) * font->bpp;
				bmap[(dy * nbpr) + (dx >> 3)] |= byte;
			}
		}
	}
}

static char *unix_eol = "\n";
static char *dos_eol = "\r\n";
static char *mac_eol = "\r";

void
#ifdef __STDC__
bdf_save_font(FILE *out, bdf_font_t *font, bdf_options_t *opts,
              bdf_callback_t callback, void *data)
#else
bdf_save_font(out, font, opts, callback, data)
FILE *out;
bdf_font_t *font;
bdf_options_t *opts;
bdf_callback_t callback;
void *data;
#endif
{
	unsigned long i, j, bpr, pcnt;
	double dw, ps, rx;
	char *sp, *ep, *eol;
	bdf_property_t *p;
	bdf_glyph_t *c, *cp, cell;
	bdf_callback_struct_t cb;

	if (font == 0)
		return;

	eol = 0;
	switch (opts->eol) {
	case BDF_UNIX_EOL:
		eol = unix_eol;
		break;
	case BDF_DOS_EOL:
		eol = dos_eol;
		break;
	case BDF_MAC_EOL:
		eol = mac_eol;
		break;
	}

	/*
	 * If the font is a character cell font, allocate some space for the
	 * bitmap.
	 */
	if (font->spacing == BDF_CHARCELL && opts->pad_cells != 0) {
		bpr = ((font->bbx.width * font->bpp) + 7) >> 3;
		cell.bytes = bpr * font->bbx.height;
		cell.bitmap = (unsigned char *) malloc(cell.bytes);
	}

	/*
	 * Emit the header.
	 */
	fprintf(out, "STARTFONT 2.1%s", eol);

	/*
	 * Emit the comments.
	 */
	if (font->comments_len > 0) {
		for (sp = font->comments; *sp; sp++) {
			ep = sp;
			while (*ep && *ep != '\n')
				ep++;
			fprintf(out, "COMMENT %.*s%s", ep - sp, sp, eol);
			sp = ep;
		}
	}

	/*
	 * Emit the font name.
	 */
	fprintf(out, "FONT %s%s", font->name, eol);

	/*
	 * Emit the size info.
	 */
	if (font->bpp == 1)
		fprintf(out, "SIZE %ld %ld %ld%s", font->point_size,
		        font->resolution_x, font->resolution_y, eol);
	else
		fprintf(out, "SIZE %ld %ld %ld %hd%s", font->point_size,
		        font->resolution_x, font->resolution_y, font->bpp, eol);

	/*
	 * Emit the bounding box.
	 */
	fprintf(out, "FONTBOUNDINGBOX %hd %hd %hd %hd%s",
	        font->bbx.width, font->bbx.height, font->bbx.x_offset,
	        font->bbx.y_offset, eol);

	/*
	 * Emit the properties after counting how many are properties and
	 * how many are comments.
	 */
	for (i = pcnt = 0, p = font->props; i < font->props_used; i++, p++) {
		if (memcmp(p->name, "COMMENT", 7) != 0)
			pcnt++;
	}

	fprintf(out, "STARTPROPERTIES %ld%s", pcnt, eol);
	for (i = 0, p = font->props; i < font->props_used; i++, p++) {
		fprintf(out, "%s ", p->name);
		if (p->format == BDF_ATOM) {
			if (p->value.atom == 0)
				fprintf(out, "\"\"%s", eol);
			else
				fprintf(out, "\"%s\"%s", p->value.atom, eol);
		} else
			fprintf(out, "%ld%s", p->value.int32, eol);
	}

	fprintf(out, "ENDPROPERTIES%s", eol);

	/*
	 * Emit the number of bitmaps in the font.
	 */
	fprintf(out, "CHARS %ld%s", font->unencoded_used + font->glyphs_used, eol);

	/*
	 * Call the callback if it was passed to start the save.
	 */
	if (callback != 0) {
		cb.reason = BDF_SAVE_START;
		cb.total = font->unencoded_used + font->glyphs_used;
		cb.current = 0;
		(*callback)(&cb, data);
	}

	/*
	 * Emit the unencoded bitmaps.
	 */
	for (i = 0, cp = font->unencoded; i < font->unencoded_used; i++, cp++) {
		/*
		 * If the font has character-cell spacing and the option to pad the
		 * glyphs to the size of the font bbx is set, then pad the glyph.
		 * Otherwise, crop the glyph to the minimum rectangle needed to hold
		 * the bitmap.
		 */
		if (font->spacing == BDF_CHARCELL && opts->pad_cells != 0) {
			/*
			 * Point at the temporary glyph structure and copy the necessary
			 * glyph info into it.
			 */
			c = &cell;
			c->name = cp->name;
			c->encoding = cp->encoding;
			c->swidth = cp->swidth;
			c->dwidth = cp->dwidth;
			(void) memcpy((char *) &c->bbx, (char *) &font->bbx,
			              sizeof(bdf_bbx_t));
			_bdf_pad_cell(font, cp, c);
		} else {
			c = cp;
			_bdf_crop_glyph(font, c);
		}

		/*
		 * If the font has monowidth or character-cell spacing, then assign
		 * the font monowidth field to the device width and recalculate the
		 * scalable width.
		 */
		if (font->spacing != BDF_PROPORTIONAL) {
			c->dwidth = font->monowidth;
			ps = (double) font->point_size;
			rx = (double) font->resolution_x;
			dw = (double) c->dwidth;
			c->swidth = (unsigned short)((dw * 72000.0) / (ps * rx));
		}
		if (c->name == 0)
			fprintf(out, "STARTCHAR unencoded%ld%sENCODING -1%s", i, eol, eol);
		else
			fprintf(out, "STARTCHAR %s%sENCODING -1%s", c->name, eol, eol);
		fprintf(out, "SWIDTH %hd 0%sDWIDTH %hd 0%s",
		        c->swidth, eol, c->dwidth, eol);
		fprintf(out, "BBX %hd %hd %hd %hd%s", c->bbx.width, c->bbx.height,
		        c->bbx.x_offset, c->bbx.y_offset, eol);
		fprintf(out, "BITMAP%s", eol);
		bpr = ((c->bbx.width * font->bpp) + 7) >> 3;
		for (j = 0; bpr != 0 && j < c->bytes; j++) {
			if (j && j % bpr == 0)
				fprintf(out, eol);
			fprintf(out, "%02X", c->bitmap[j]);
		}
		/*
		 * Handle empty bitmaps like this.
		 */
		if (c->bbx.height > 0)
			fprintf(out, eol);
		fprintf(out, "ENDCHAR%s", eol);

		/*
		 * Call the callback if supplied.
		 */
		if (callback != 0) {
			cb.reason = BDF_SAVING;
			cb.current++;
			(*callback)(&cb, data);
		}
	}

	/*
	 * Emit the other bitmaps.
	 */
	for (i = 0, cp = font->glyphs; i < font->glyphs_used; i++, cp++) {
		/*
		 * If the font has character-cell spacing and the option to pad the
		 * glyphs to the size of the font bbx is set, then pad the glyph.
		 * Otherwise, crop the glyph to the minimum rectangle needed to hold
		 * the bitmap.
		 */
		if (font->spacing == BDF_CHARCELL && opts->pad_cells != 0) {
			/*
			 * Point at the temporary glyph structure and copy the necessary
			 * glyph info into it.
			 */
			c = &cell;
			c->name = cp->name;
			c->encoding = cp->encoding;
			c->swidth = cp->swidth;
			c->dwidth = cp->dwidth;
			(void) memcpy((char *) &c->bbx, (char *) &font->bbx,
			              sizeof(bdf_bbx_t));
			_bdf_pad_cell(font, cp, c);
		} else {
			c = cp;
			_bdf_crop_glyph(font, c);
		}

		/*
		 * If the font has monowidth or character-cell spacing, then assign
		 * the font monowidth field to the device width and recalculate the
		 * scalable width.
		 */
		if (font->spacing != BDF_PROPORTIONAL) {
			c->dwidth = font->monowidth;
			ps = (double) font->point_size;
			rx = (double) font->resolution_x;
			dw = (double) c->dwidth;
			c->swidth = (unsigned short)((dw * 72000.0) / (ps * rx));
		}
		if (c->name == 0)
			fprintf(out, "STARTCHAR char%ld%sENCODING %ld%s",
			        c->encoding, eol, c->encoding, eol);
		else
			fprintf(out, "STARTCHAR %s%sENCODING %ld%s",
			        c->name, eol, c->encoding, eol);
		fprintf(out, "SWIDTH %hd 0%sDWIDTH %hd 0%s",
		        c->swidth, eol, c->dwidth, eol);
		fprintf(out, "BBX %hd %hd %hd %hd%s", c->bbx.width, c->bbx.height,
		        c->bbx.x_offset, c->bbx.y_offset, eol);
		fprintf(out, "BITMAP%s", eol);
		bpr = ((c->bbx.width * font->bpp) + 7) >> 3;
		for (j = 0; bpr != 0 && j < c->bytes; j++) {
			if (j && j % bpr == 0)
				fprintf(out, eol);
			fprintf(out, "%02X", c->bitmap[j]);
		}
		/*
		 * Handle empty bitmaps like this.
		 */
		if (c->bbx.height > 0)
			fprintf(out, eol);
		fprintf(out, "ENDCHAR%s", eol);

		/*
		 * Call the callback if supplied.
		 */
		if (callback != 0) {
			cb.reason = BDF_SAVING;
			cb.current++;
			(*callback)(&cb, data);
		}
	}

	/*
	 * Emit the trailer.
	 */
	fprintf(out, "ENDFONT%s", eol);

	/*
	 * Always force a final call to the callback to make sure things
	 * get cleaned up.
	 */
	if (callback != 0) {
		cb.reason = BDF_SAVING;
		cb.current = cb.total;
		(*callback)(&cb, data);
	}

	/*
	 * If the font is a character cell font, clean up the temporary glyph.
	 */
	if (font->spacing == BDF_CHARCELL && opts->pad_cells != 0)
		free((char *) cell.bitmap);
}

/*
 * Routine to write a single set of SBIT metrics.
 */
void
#ifdef __STDC__
bdf_save_sbit_metrics(FILE *out, bdf_font_t *font, bdf_options_t *opts,
                      char *appname)
#else
bdf_save_sbit_metrics(out, font, opts, appname)
bdf_font_t *font;
bdf_options_t *opts;
char *appname;
#endif
{
	char *eol;

	eol = 0;
	switch (opts->eol) {
	case BDF_UNIX_EOL:
		eol = unix_eol;
		break;
	case BDF_DOS_EOL:
		eol = dos_eol;
		break;
	case BDF_MAC_EOL:
		eol = mac_eol;
		break;
	}

	/*
	 * Throw a simple header in.
	 */
	if (appname)
		fprintf(out, ";%s; SBIT metrics file generated by \"%s\".%s;%s%s", eol,
		        appname, eol, eol, eol);

	/*
	 * Save PPEM.
	 */
	fprintf(out, ";%s; Pixels Per Em.%s;%s", eol, eol, eol);
	fprintf(out, "PPEM %ld%s%s", font->point_size, eol, eol);

	/*
	 * If the font is character cell or monowidth, set this boolean.
	 */
	if (font->spacing != BDF_PROPORTIONAL) {
		fprintf(out,
		        ";%s; Font is not proportional, so use mono advance.%s;%s",
		        eol, eol, eol);
		fprintf(out, "FORCECONSTANTMETRICS TRUE%s%s", eol, eol);
	} else {
		fprintf(out,
		        ";%s; Font is proportional, so do not use mono advance.%s;%s",
		        eol, eol, eol);
		fprintf(out, "FORCECONSTANTMETRICS FALSE%s%s", eol, eol);
	}

	/*
	 * Do the horizontal line metrics only.
	 */
	fprintf(out, ";%s; Horizontal line metrics.%s;%s", eol, eol, eol);

	fprintf(out, "H_ASCENDER %ld%sH_DESCENDER %ld%s", font->font_ascent, eol,
	        font->font_descent, eol);
	fprintf(out, "H_WIDTHMAX %hd%s", font->bbx.width, eol);
	fprintf(out, "H_MINORIGINSB %hd%sH_MINADVANCEBL %hd%s",
	        font->bbx.x_offset, eol,
	        font->bbx.width + font->bbx.x_offset, eol);
	fprintf(out, "H_MAXBEFOREBL %hd%sH_MINAFTERBL %hd%s%s",
	        font->bbx.ascent, eol, font->bbx.y_offset, eol, eol);

	/*
	 * Write the default caret info.
	 */
	fprintf(out, ";%s; Caret slope and offset info.%s;%s", eol, eol, eol);
	fprintf(out, "CARETSLOPENUMERATOR 1%sCARETSLOPEDENOMINATOR 0%s", eol, eol);
	fprintf(out, "CARETOFFSET 0%s%s", eol, eol);

	/*
	 * Write the bitmap options.
	 */
	fprintf(out, ";%s; Bitmap options.%s;%s", eol, eol, eol);
	fprintf(out, "DIRECTION H%sSTORAGE FAST%s%s", eol, eol, eol);

	/*
	 * Scaled bitmaps not implemented yet.
	 */
	fprintf(out, ";%s; Scaled bitmap info (Not Yet Implemented).%s;%s",
	        eol, eol, eol);
}

/*
 * Special routine to dump the font in the Roman Czyborra's hex format.  It
 * only dumps the encoded glyphs and assumes the bitmaps have the correct
 * sizes.
 */
void
#ifdef __STDC__
bdf_export_hex(FILE *out, bdf_font_t *font, bdf_callback_t callback,
               void *data)
#else
bdf_export_hex(out, font, callback, data)
FILE *out;
bdf_font_t *font;
bdf_callback_t callback;
void *data;
#endif
{
	int bpr, fbpr, j, k;
	unsigned long i, ng;
	bdf_glyph_t *gp, cell;
	bdf_callback_struct_t cb;

	if (font == 0 || out == 0)
		return;

	if (font->glyphs_used == 0)
		return;

	/*
	 * Call the callback if it was passed to start the export.
	 */
	if (callback != 0) {
		cb.reason = BDF_EXPORT_START;
		cb.total = font->glyphs_used;
		cb.current = 0;
		(*callback)(&cb, data);
	}

	fbpr = ((font->bbx.width * font->bpp) + 7) >> 3;
	bpr = (((font->bbx.width >> 1) * font->bpp) + 7) >> 3;
	cell.bytes = fbpr * font->bbx.height;
	cell.bitmap = (unsigned char *) malloc(cell.bytes);

	for (i = 0, ng = font->glyphs_used, gp = font->glyphs; i < ng; i++, gp++) {
		_bdf_pad_cell(font, gp, &cell);
		fprintf(out, "%04lX:", gp->encoding & 0xffff);
		if (gp->bbx.width <= (font->bbx.width >> 1)) {
			for (j = 0; j < cell.bytes; j += fbpr) {
				for (k = 0; k < bpr; k++)
					fprintf(out, "%02X", cell.bitmap[j + k]);
			}
		} else {
			for (j = 0; j < cell.bytes; j++)
				fprintf(out, "%02X", cell.bitmap[j]);
		}
		if (cell.bytes > 0)
			putc('\n', out);

		/*
		 * Call the callback if supplied.
		 */
		if (callback != 0) {
			cb.reason = BDF_EXPORTING;
			cb.current++;
			(*callback)(&cb, data);
		}
	}

	/*
	 * Clean up the cell.
	 */
	free((char *) cell.bitmap);

	/*
	 * Always call a final callback to make sure the client gets a chance to
	 * clean things up.
	 */
	if (callback != 0) {
		cb.reason = BDF_EXPORTING;
		cb.current = cb.total;
		(*callback)(&cb, data);
	}
}

void
#ifdef __STDC__
bdf_free_font(bdf_font_t *font)
#else
bdf_free_font(font)
bdf_font_t *font;
#endif
{
	unsigned long i;
	bdf_glyph_t *glyphs;

	if (font == 0)
		return;

	if (font->name != 0)
		free(font->name);

	/*
	 * Free up the internal hash table of property names.
	 */
	hash_free((hashtable *) font->internal);
	free((char *) font->internal);

	/*
	 * Free up the comment info.
	 */
	if (font->comments_len > 0)
		free(font->comments);

	/*
	 * Free up the auto-correction messages.
	 */
	if (font->acmsgs_len > 0)
		free(font->acmsgs);

	/*
	 * Free up the properties.
	 */
	for (i = 0; i < font->props_size; i++) {
		if (font->props[i].format == BDF_ATOM && font->props[i].value.atom)
			free(font->props[i].value.atom);
	}

	if (font->props_size > 0 && font->props != 0)
		free((char *) font->props);

	/*
	 * Free up the character info.
	 */
	for (i = 0, glyphs = font->glyphs; i < font->glyphs_used; i++, glyphs++) {
		if (glyphs->name)
			free(glyphs->name);
		if (glyphs->bytes > 0 && glyphs->bitmap != 0)
			free((char *) glyphs->bitmap);
	}

	for (i = 0, glyphs = font->unencoded; i < font->unencoded_used;
	        i++, glyphs++) {
		if (glyphs->name)
			free(glyphs->name);
		if (glyphs->bytes > 0)
			free((char *) glyphs->bitmap);
	}

	if (font->glyphs_size > 0)
		free((char *) font->glyphs);

	if (font->unencoded_size > 0)
		free((char *) font->unencoded);

	/*
	 * Free up the overflow storage if it was used.
	 */
	for (i = 0, glyphs = font->overflow.glyphs; i < font->overflow.glyphs_used;
	        i++, glyphs++) {
		if (glyphs->name != 0)
			free(glyphs->name);
		if (glyphs->bytes > 0)
			free((char *) glyphs->bitmap);;
	}
	if (font->overflow.glyphs_size > 0)
		free((char *) font->overflow.glyphs);

	free((char *) font);
}

void
#ifdef __STDC__
bdf_create_property(char *name, int format)
#else
bdf_create_property(name, format)
char *name;
int format;
#endif
{
	unsigned long n;
	bdf_property_t *p;

	/*
	 * First check to see if the property has
	 * already been added or not.  If it has, then
	 * simply ignore it.
	 */

	if (hash_lookup(name, &proptbl))
		return;

	if (nuser_props == 0)
		user_props = (bdf_property_t *) malloc(sizeof(bdf_property_t));
	else
		user_props = (bdf_property_t *) realloc((char *) user_props,
		                                        sizeof(bdf_property_t) *
		                                        (nuser_props + 1));

	p = user_props + nuser_props;
	(void) memset((char *) p, 0, sizeof(bdf_property_t));
	n = (unsigned long)(strlen(name) + 1);
	p->name = (char *) malloc(n);
	(void) memcpy(p->name, name, n);
	p->format = format;
	p->builtin = 0;

	n = _num_bdf_properties + nuser_props;
	hash_insert(p->name, (void *) n, &proptbl);

	nuser_props++;
}

bdf_property_t *
#ifdef __STDC__
bdf_get_property(char *name)
#else
bdf_get_property(name)
char *name;
#endif
{
	hashnode hn;
	unsigned long propid;

	if (name == 0 || *name == 0)
		return 0;

	if ((hn = hash_lookup(name, &proptbl)) == 0)
		return 0;

	propid = (unsigned long) hn->data;
	if (propid >= _num_bdf_properties)
		return user_props + (propid - _num_bdf_properties);
	return _bdf_properties + propid;
}

/*
 * Routine to compare two property names.
 */
static int
#ifdef __STDC__
by_prop_name(const void *a, const void *b)
#else
by_prop_name(a, b)
char *a, *b;
#endif
{
	bdf_property_t *p1, *p2;

	p1 = (bdf_property_t *) a;
	p2 = (bdf_property_t *) b;

	return strcmp(p1->name, p2->name);
}

unsigned long
#ifdef __STDC__
bdf_property_list(bdf_property_t **props)
#else
bdf_property_list(props)
bdf_property_t **props;
#endif
{
	unsigned long n;
	bdf_property_t *p;

	n = _num_bdf_properties + nuser_props;
	if (props != 0 && n != 0) {
		p = (bdf_property_t *) malloc(sizeof(bdf_property_t) * n);
		(void) memcpy((char *) p, (char *) _bdf_properties,
		              sizeof(bdf_property_t) * _num_bdf_properties);
		(void) memcpy((char *)(p + _num_bdf_properties), (char *) user_props,
		              sizeof(bdf_property_t) * nuser_props);
		qsort((char *) p, n, sizeof(bdf_property_t), by_prop_name);
		*props = p;
	}
	return n;
}

int
#ifdef __STDC__
bdf_replace_comments(bdf_font_t *font, char *comments,
                     unsigned long comments_len)
#else
bdf_replace_comments(font, comments, comments_len)
bdf_font_t *font;
char *comments;
unsigned long comments_len;
#endif
{
	if (font == 0 || comments_len == 0)
		return 0;

	if (font->comments_len > 0)
		free(font->comments);

	font->comments = (char *) malloc(comments_len + 1);
	(void) memcpy(font->comments, comments, comments_len);
	font->comments[comments_len] = 0;
	font->comments_len = comments_len;
	font->modified = 1;
	return 1;
}

unsigned long
#ifdef __STDC__
bdf_font_property_list(bdf_font_t *font, bdf_property_t **props)
#else
bdf_font_property_list(font, props)
bdf_font_t *font;
bdf_property_t **props;
#endif
{
	bdf_property_t *p;

	if (font == 0 || font->props_used == 0)
		return 0;

	if (props != 0) {
		p = (bdf_property_t *) malloc(sizeof(bdf_property_t) *
		                              font->props_used);
		(void) memcpy((char *) p, (char *) font->props,
		              sizeof(bdf_property_t) * font->props_used);
		qsort((char *) p, font->props_used, sizeof(bdf_property_t),
		      by_prop_name);
		*props = p;
	}

	return font->props_used;
}

void
#ifdef __STDC__
bdf_add_font_property(bdf_font_t *font, bdf_property_t *property)
#else
bdf_add_font_property(font, property)
bdf_font_t *font;
bdf_property_t *property;
#endif
{
	int len;
	unsigned long propid;
	hashnode hn;
	bdf_property_t *p, *ip;

	if (property == 0 || property->name == 0 || property->name[0] == 0)
		return;

	/*
	 * If the font does not have a property hash table yet, make
	 * sure it is allocated.
	 */
	if (font->internal == 0) {
		font->internal = (void *) malloc(sizeof(hashtable));
		hash_init((hashtable *) font->internal);
	}

	/*
	 * See if the property is in the general property table yet.
	 * If it isn't, then add it.
	 */
	if ((hn = hash_lookup(property->name, &proptbl)) == 0)
		bdf_create_property(property->name, property->format);
	else {
		/*
		 * If the property exists and is a user defined property, make sure
		 * its format is updated to match the property being added.
		 */
		propid = (unsigned long) hn->data;
		if (propid >= _num_bdf_properties) {
			p = user_props + (propid - _num_bdf_properties);
			if (p->format != property->format)
				p->format = property->format;
		}
	}

	/*
	 * If the font already has this property, then change the existing one.
	 */
	hn = hash_lookup(property->name, (hashtable *) font->internal);
	if (hn != 0) {
		/*
		 * Changing an existing property value.
		 */
		p = font->props + ((unsigned long) hn->data);

		/*
		 * If the format changed, then free the atom value if the original
		 * format was an atom.
		 */
		if (p->format == BDF_ATOM && property->format != BDF_ATOM &&
		        p->value.atom != 0)
			free((char *) p->value.atom);
		p->format = property->format;

		switch (p->format) {
		case BDF_ATOM:
			/*
			 * If the property value is the same, then just return.
			 */
			if (property->value.atom == p->value.atom ||
			        (property->value.atom && p->value.atom &&
			         strcmp(property->value.atom, p->value.atom) == 0))
				return;
			if (property->value.atom == 0)
				len = 1;
			else
				len = strlen(property->value.atom) + 1;
			if (len > 1) {
				p->value.atom = (char *) malloc(len);
				(void) memcpy(p->value.atom, property->value.atom, len);
			} else
				p->value.atom = 0;
			break;
		case BDF_INTEGER:
			/*
			 * If the property value is the same, then just return.
			 */
			if (p->value.int32 == property->value.int32)
				return;
			p->value.int32 = property->value.int32;
			break;
		case BDF_CARDINAL:
			/*
			 * If the property value is the same, then just return.
			 */
			if (p->value.card32 == property->value.card32)
				return;
			p->value.card32 = property->value.card32;
			break;
		}
	} else {
		/*
		 * New property being added.
		 */

		/*
		 * Get the internal table entry for a pointer to the
		 * name of the property.
		 */
		hn = hash_lookup(property->name, &proptbl);
		propid = (unsigned long) hn->data;
		if (propid >= _num_bdf_properties)
			ip = user_props + (propid - _num_bdf_properties);
		else
			ip = _bdf_properties + propid;

		/*
		 * Add it to the property list first.
		 */
		if (font->props_used == font->props_size) {
			if (font->props_size == 0)
				font->props = (bdf_property_t *) malloc(sizeof(bdf_property_t));
			else
				font->props = (bdf_property_t *)
				              realloc((char *) font->props, sizeof(bdf_property_t) *
				                      (font->props_size + 1));
			font->props_size++;
		}
		p = font->props + font->props_used;

		p->name = ip->name;
		p->format = ip->format;
		p->builtin = ip->builtin;

		switch (p->format) {
		case BDF_ATOM:
			if (property->value.atom == 0)
				len = 1;
			else
				len = strlen(property->value.atom) + 1;
			if (len > 1) {
				p->value.atom = (char *) malloc(len);
				(void) memcpy(p->value.atom, property->value.atom, len);
			} else
				p->value.atom = 0;
			break;
		case BDF_INTEGER:
			p->value.int32 = property->value.int32;
			break;
		case BDF_CARDINAL:
			p->value.card32 = property->value.card32;
			break;
		}

		/*
		 * Now insert it into the internal hash table.
		 */
		hash_insert(p->name, (void *) font->props_used,
		            (hashtable *) font->internal);
		font->props_used++;
	}

	if (memcmp(property->name, "DEFAULT_CHAR", 12) == 0)
		/*
		 * If the property just added is DEFAULT_CHAR, then make sure the
		 * default_glyph field is set.
		 */
		font->default_glyph = p->value.card32;
	else if (memcmp(property->name, "FONT_ASCENT", 11) == 0)
		/*
		 * If the property just added is FONT_ASCENT, then adjust the
		 * font_ascent field.
		 */
		font->font_ascent = p->value.int32;
	else if (memcmp(property->name, "FONT_DESCENT", 12) == 0)
		/*
		 * If the property just added is FONT_DESCENT, then adjust the
		 * font_descent field.
		 */
		font->font_descent = p->value.int32;
	else if (memcmp(property->name, "RESOLUTION_X", 12) == 0)
		/*
		 * If the property just added is RESOLUTION_X, then adjust the
		 * resolution_x field.
		 */
		font->resolution_x = p->value.card32;
	else if (memcmp(property->name, "RESOLUTION_Y", 12) == 0)
		/*
		 * If the property just added is RESOLUTION_Y, then adjust the
		 * resolution_y field.
		 */
		font->resolution_y = p->value.card32;
	else if (memcmp(property->name, "POINT_SIZE", 10) == 0)
		/*
		 * If the property just added is POINT_SIZE, then adjust the
		 * point_size field.
		 */
		font->point_size = p->value.int32 / 10;
	else if (memcmp(property->name, "SPACING", 7) == 0) {
		/*
		 * Make sure the font spacing is kept in synch if the property
		 * changes.  If the spacing changes from proportional to one
		 * of the others, force the monowidth to be set.
		 */
		switch (p->value.atom[0]) {
		case 'C':
		case 'c':
			if (font->spacing == BDF_PROPORTIONAL)
				font->monowidth = font->bbx.width + font->bbx.x_offset;
			font->spacing = BDF_CHARCELL;
			break;
		case 'M':
		case 'm':
			if (font->spacing == BDF_PROPORTIONAL)
				font->monowidth = font->bbx.width + font->bbx.x_offset;
			font->spacing = BDF_MONOWIDTH;
			break;
		case 'P':
		case 'p':
			font->spacing = BDF_PROPORTIONAL;
			break;
		}
	}

	/*
	 * Make sure the font is marked as modified.
	 */
	font->modified = 1;
}

void
#ifdef __STDC__
bdf_delete_font_property(bdf_font_t *font, char *name)
#else
bdf_delete_font_property(font, name)
bdf_font_t *font;
char *name;
#endif
{
	hashnode hn;
	unsigned long off;
	bdf_property_t *p;

	if (font == 0 || name == 0 || *name == 0 || font->props_used == 0)
		return;

	if ((hn = hash_lookup(name, (hashtable *) font->internal)) == 0)
		return;

	off = (unsigned long) hn->data;
	p = font->props + off;

	/*
	 * Delete the ATOM value if appropriate.
	 */
	if (p->format == BDF_ATOM && p->value.atom != 0)
		free(p->value.atom);

	/*
	 * The property exists.  Two things needs to be done:
	 * 1. Remove the property from the hash table.
	 * 2. Remove the property from the font's list of properties.
	 */
	hash_delete(name, (hashtable *) font->internal);

	/*
	 * Locate its offset in the font property list.
	 */
	if (off < font->props_used - 1)
		/*
		 * We have to shift the property list down.
		 */
		_bdf_memmove((char *) p, (char *)(p + 1),
		             sizeof(bdf_property_t) * ((font->props_used - 1) - off));
	font->props_used--;

	/*
	 * If the font property happens to be DEFAULT_CHAR, then make sure the
	 * default_glyph field is reset.
	 */
	if (strncmp(name, "DEFAULT_CHAR", 12) == 0)
		font->default_glyph = -1;

	/*
	 * Update the hash table with the correct indexes.
	 */
	for (off = 0, p = font->props; off < font->props_used; off++, p++)
		hash_insert(p->name, (void *) off, (hashtable *) font->internal);

	/*
	 * Mark the font as being modified.
	 */
	font->modified = 1;
}

bdf_property_t *
#ifdef __STDC__
bdf_get_font_property(bdf_font_t *font, const char *name)
#else
bdf_get_font_property(font, name)
bdf_font_t *font;
const char *name;
#endif
{
	hashnode hn;

	if (font == 0 || font->props_size == 0 || name == 0 || *name == 0)
		return 0;

	hn = hash_lookup((char*)name, (hashtable *) font->internal);
	return (hn) ? (font->props + ((unsigned long) hn->data)) : 0;
}

typedef struct {
	bdf_options_t *opts;
	bdf_options_callback_t callback;
	void *client_data;
	_bdf_list_t list;
} _bdf_opts_parse_t;

static int
#ifdef __STDC__
_bdf_get_boolean(char *val)
#else
_bdf_get_boolean(val)
char *val;
#endif
{
	int ok;

	ok = 0;
	if (val == 0 || *val == 0)
		return ok;

	switch (val[0]) {
	case '0':
	case 'F':
	case 'f':
	case 'N':
	case 'n':
		ok = 0;
		break;
	case '1':
	case 'T':
	case 't':
	case 'Y':
	case 'y':
		ok = 1;
		break;
	}
	return ok;
}

static int
#ifdef __STDC__
_bdf_parse_options(char *line, unsigned long linelen, unsigned long lineno,
                   void *call_data, void *client_data)
#else
_bdf_parse_options(line, linelen, lineno, call_data, client_data)
char *line;
unsigned long linelen, lineno;
void *call_data, *client_data;
#endif
{
	_bdf_list_t *lp;
	_bdf_opts_parse_t *p;
	long bpp;

	p = (_bdf_opts_parse_t *) client_data;
	lp = &p->list;

	/*
	 * Split the line into fields.
	 */
	_bdf_split(" \t+", line, linelen, lp);

	if (lp->field[0][0] == 'b' &&
	        memcmp(lp->field[0], "bits_per_pixel", 14) == 0) {
		if (lp->used < 2) {
			fprintf(stderr,
			        "bdf: warning: %ld: incorrect number of fields %ld.\n",
			        lineno, lp->used);
			fprintf(stderr,
			        "bdf: warning: %ld: bits_per_pixel <1, 2, or 4>.\n",
			        lineno);
		} else {
			bpp = _bdf_atol(lp->field[1], 0, 10);
			if (!(bpp == 1 || bpp == 2 || bpp == 4)) {
				fprintf(stderr,
				        "bdf: warning: %ld: invalid bits per pixel %ld.\n",
				        lineno, bpp);
				fprintf(stderr,
				        "bdf: warning: %ld: bits_per_pixel <1, 2, or 4>.\n",
				        lineno);
			} else
				p->opts->bits_per_pixel = bpp;
		}
		return 0;
	}

	if (lp->field[0][0] == 'e' && memcmp(lp->field[0], "eol", 3) == 0) {
		if (lp->used < 2) {
			fprintf(stderr,
			        "bdf: warning: %ld: incorrect number of fields %ld.\n",
			        lineno, lp->used);
			fprintf(stderr,
			        "bdf: warning: %ld: eol <eolname>.\n", lineno);
		} else {
			switch (lp->field[1][0]) {
			case 'u':
			case 'U':
				p->opts->eol = BDF_UNIX_EOL;
				break;
			case 'd':
			case 'D':
				p->opts->eol = BDF_DOS_EOL;
				break;
			case 'm':
			case 'M':
				p->opts->eol = BDF_MAC_EOL;
				break;
			}
		}
		return 0;
	}

	if (lp->field[0][0] == 'c' &&
	        memcmp(lp->field[0], "correct_metrics", 15) == 0) {
		if (lp->used < 2) {
			fprintf(stderr,
			        "bdf: warning: %ld: incorrect number of fields %ld.\n",
			        lineno, lp->used);
			fprintf(stderr,
			        "bdf: warning: %ld: correct_metrics <boolean>.\n", lineno);
		} else
			p->opts->correct_metrics = _bdf_get_boolean(lp->field[1]);

		return 0;
	}

	if (lp->field[0][0] == 'k' &&
	        memcmp(lp->field[0], "keep_unencoded", 14) == 0) {
		if (lp->used < 2) {
			fprintf(stderr,
			        "bdf: warning: %ld: incorrect number of fields %ld.\n",
			        lineno, lp->used);
			fprintf(stderr,
			        "bdf: warning: %ld: keep_unencoded <boolean>.\n", lineno);
		} else
			p->opts->keep_unencoded = _bdf_get_boolean(lp->field[1]);

		return 0;
	}

	if (lp->field[0][0] == 'k' &&
	        memcmp(lp->field[0], "keep_comments", 13) == 0) {
		if (lp->used < 2) {
			fprintf(stderr,
			        "bdf: warning: %ld: incorrect number of fields %ld.\n",
			        lineno, lp->used);
			fprintf(stderr,
			        "bdf: warning: %ld: keep_comments <boolean>.\n", lineno);
		} else
			p->opts->keep_comments = _bdf_get_boolean(lp->field[1]);

		return 0;
	}

	if (lp->field[0][0] == 'p' &&
	        memcmp(lp->field[0], "pad_character_cells", 19) == 0) {
		if (lp->used < 2) {
			fprintf(stderr,
			        "bdf: warning: %ld: incorrect number of fields %ld.\n",
			        lineno, lp->used);
			fprintf(stderr,
			        "bdf: warning: %ld: pad_character_cells <boolean>.\n",
			        lineno);
		} else
			p->opts->pad_cells = _bdf_get_boolean(lp->field[1]);

		return 0;
	}

	if (lp->field[0][0] == 'p' &&
	        memcmp(lp->field[0], "point_size", 10) == 0) {
		if (lp->used < 2) {
			fprintf(stderr,
			        "bdf: warning: %ld: incorrect number of fields %ld.\n",
			        lineno, lp->used);
			fprintf(stderr,
			        "bdf: warning: %ld: point_size <integer>.\n", lineno);
		} else
			p->opts->point_size = _bdf_atol(lp->field[1], 0, 10);
		return 0;
	}

	if (lp->field[0][0] == 'h' &&
	        memcmp(lp->field[0], "horizontal_resolution", 21) == 0) {
		if (lp->used < 2) {
			fprintf(stderr,
			        "bdf: warning: %ld: incorrect number of fields %ld.\n",
			        lineno, lp->used);
			fprintf(stderr,
			        "bdf: warning: %ld: horizontal_resolution <cardinal>.\n",
			        lineno);
		} else
			p->opts->resolution_x = _bdf_atoul(lp->field[1], 0, 10);
		return 0;
	}

	if (lp->field[0][0] == 'v' &&
	        memcmp(lp->field[0], "vertical_resolution", 19) == 0) {
		if (lp->used < 2) {
			fprintf(stderr,
			        "bdf: warning: %ld: incorrect number of fields %ld.\n",
			        lineno, lp->used);
			fprintf(stderr,
			        "bdf: warning: %ld: vertical_resolution <cardinal>.\n",
			        lineno);
		} else
			p->opts->resolution_y = _bdf_atoul(lp->field[1], 0, 10);
		return 0;
	}

	if (lp->field[0][0] == 'f' &&
	        memcmp(lp->field[0], "font_spacing", 12) == 0) {
		if (lp->used < 2) {
			fprintf(stderr,
			        "bdf: warning: %ld: incorrect number of fields %ld.\n",
			        lineno, lp->used);
			fprintf(stderr,
			        "bdf: warning: %ld: font_spacing <spacing name>.\n",
			        lineno);
		} else {
			switch (lp->field[1][0]) {
			case 'P':
			case 'p':
				p->opts->font_spacing = BDF_PROPORTIONAL;
				break;
			case 'M':
			case 'm':
				p->opts->font_spacing = BDF_MONOWIDTH;
				break;
			case 'C':
			case 'c':
				p->opts->font_spacing = BDF_CHARCELL;
				break;
			default:
				fprintf(stderr,
				        "bdf: warning: %ld: unknown font spacing '%s'.\n",
				        lineno, lp->field[1]);
			}
		}
		return 0;
	}

	if (lp->field[0][0] == 'p' &&
	        memcmp(lp->field[0], "property", 8) == 0) {
		if (lp->used < 3) {
			fprintf(stderr,
			        "bdf: warning: %ld: incorrect number of fields %ld.\n",
			        lineno, lp->used);
			fprintf(stderr,
			        "bdf: warning: %ld: property <name> <type>.\n",
			        lineno);
		} else {
			switch (lp->field[2][0]) {
			case 'A':
			case 'a':
				bdf_create_property(lp->field[1], BDF_ATOM);
				break;
			case 'C':
			case 'c':
				bdf_create_property(lp->field[1], BDF_CARDINAL);
				break;
			case 'I':
			case 'i':
				bdf_create_property(lp->field[1], BDF_INTEGER);
				break;
			default:
				fprintf(stderr,
				        "bdf: warning: %ld: unknown property type '%s'.\n",
				        lineno, lp->field[2]);
			}
		}
		return 0;
	}

	if (lp->field[0][0] == 'h' &&
	        memcmp(lp->field[0], "hint_truetype_glyphs", 20) == 0) {
		if (lp->used < 2) {
			fprintf(stderr,
			        "bdf: warning: %ld: incorrect number of fields %ld.\n",
			        lineno, lp->used);
			fprintf(stderr,
			        "bdf: warning: %ld: hint_truetype_glyphs <boolean>.\n",
			        lineno);
		} else
			p->opts->ttf_hint = _bdf_get_boolean(lp->field[1]);

		return 0;
	}

	if (lp->field[0][0] == 'g' &&
	        memcmp(lp->field[0], "generate_ranges", 15) == 0)
		/*
		 * Simply ignore the glyph ranges entry in the config file.
		 */
		return 0;

	/*
	 * If the callback returns a non-zero value, the caller has handled the
	 * unknown option found in the file.
	 */
	if (p->callback != 0 &&
	        (*p->callback)(p->opts, lp->field, lp->used, p->client_data) != 0)
		return 0;

	fprintf(stderr, "bdf: warning: %ld: unknown configuration option '%s'.\n",
	        lineno, lp->field[0]);
	return 0;
}

void
#ifdef __STDC__
bdf_load_options(FILE *in, bdf_options_t *opts,
                 bdf_options_callback_t callback, void *client_data)
#else
bdf_load_options(in, opts, callback, client_data)
FILE *in;
bdf_options_t *opts;
bdf_options_callback_t callback;
void *client_data;
#endif
{
	unsigned long lineno;
	_bdf_opts_parse_t p;

	/*
	 * Don't bother loading the options if the file or options structure
	 * is NULL.
	 */
	if (in == 0 || opts == 0)
		return;

	(void *) memset((char *) &p, 0, sizeof(_bdf_opts_parse_t));
	p.opts = opts;
	p.callback = callback;
	p.client_data = client_data;
	(void) _bdf_readlines(fileno(in), _bdf_parse_options, (void *) &p,
	                      &lineno);

	/*
	 * Free up the list if there is any space allocated.
	 */
	if (p.list.size > 0)
		free((char *) p.list.field);
}

void
#ifdef __STDC__
bdf_save_options(FILE *out, bdf_options_t *opts)
#else
bdf_save_options(out, opts)
FILE *out;
bdf_options_t *opts;
#endif
{
	unsigned long i;

	if (out == 0 || opts == 0)
		return;

	fprintf(out, "#\n# Metrics corrections.\n#\ncorrect_metrics ");
	if (opts->correct_metrics)
		fprintf(out, "true\n\n");
	else
		fprintf(out, "false\n\n");

	fprintf(out, "#\n# Preserve unencoded glyphs.\n#\nkeep_unencoded ");
	if (opts->keep_unencoded)
		fprintf(out, "true\n\n");
	else
		fprintf(out, "false\n\n");

	fprintf(out, "#\n# Preserve comments.\n#\nkeep_comments ");
	if (opts->keep_comments)
		fprintf(out, "true\n\n");
	else
		fprintf(out, "false\n\n");

	fprintf(out, "#\n# Pad character cells.\n#\npad_character_cells ");
	if (opts->pad_cells)
		fprintf(out, "true\n\n");
	else
		fprintf(out, "false\n\n");

	fprintf(out, "#\n# Font spacing.\n#\nfont_spacing ");
	switch (opts->font_spacing) {
	case BDF_PROPORTIONAL:
		fprintf(out, "proportional\n\n");
		break;
	case BDF_MONOWIDTH:
		fprintf(out, "monowidth\n\n");
		break;
	case BDF_CHARCELL:
		fprintf(out, "charactercell\n\n");
		break;
	}

	fprintf(out, "#\n# Point size.\n#\npoint_size %ld\n\n", opts->point_size);

	fprintf(out,
	        "#\n# Horizontal resolution.\n#\nhorizontal_resolution %ld\n\n",
	        opts->resolution_x);

	fprintf(out,
	        "#\n# Vertical resolution.\n#\nvertical_resolution %ld\n\n",
	        opts->resolution_x);

	fprintf(out,
	        "#\n# Bits per pixel.\n#\nbits_per_pixel %d\n\n",
	        opts->bits_per_pixel);

	fprintf(out, "#\n# Hint TrueType glyphs.\n#\nhint_truetype_glyphs ");
	if (opts->ttf_hint)
		fprintf(out, "true\n\n");
	else
		fprintf(out, "false\n\n");

	fprintf(out, "#\n# Set the EOL used when writing BDF fonts.\n#\neol ");
	switch (opts->eol) {
	case BDF_UNIX_EOL:
		fprintf(out, "unix\n\n");
		break;
	case BDF_DOS_EOL:
		fprintf(out, "dos\n\n");
		break;
	case BDF_MAC_EOL:
		fprintf(out, "mac\n\n");
		break;
	}

	/*
	 * Write out the user defined properties if they exist.
	 */
	if (nuser_props == 0)
		return;

	fprintf(out, "#\n# User defined properties.\n#\n");

	for (i = 0; i < nuser_props; i++) {
		fprintf(out, "property %s ", user_props[i].name);
		switch (user_props[i].format) {
		case BDF_ATOM:
			fprintf(out, "atom\n");
			break;
		case BDF_CARDINAL:
			fprintf(out, "cardinal\n");
			break;
		case BDF_INTEGER:
			fprintf(out, "integer\n");
			break;
		}
	}
}

void
#ifdef __STDC__
bdf_default_options(bdf_options_t *opts)
#else
bdf_default_options(opts)
bdf_options_t *opts;
#endif
{
	if (opts == 0)
		return;

	(void) memcpy((char *) opts, (char *) &_bdf_opts, sizeof(bdf_options_t));
}

bdf_font_t *
#ifdef __STDC__
bdf_new_font(char *name, long point_size, long resolution_x, long resolution_y,
             long spacing, int bpp)
#else
bdf_new_font(name, point_size, resolution_x, resolution_y, spacing, bpp)
char *name;
long point_size, resolution_x, resolution_y, spacing;
int bpp;
#endif
{
	long psize;
	char sp[2];
	bdf_font_t *font;
	double dp, dr;
	bdf_property_t prop;

	font = (bdf_font_t *) calloc(1, sizeof(bdf_font_t));
	if (name != 0 && *name != 0) {
		font->name = (char *) malloc(strlen(name) + 1);
		(void) strcpy(font->name, name);
	}

	font->bpp = bpp;
	font->point_size = point_size;
	font->resolution_x = resolution_x;
	font->resolution_y = resolution_y;

	/*
	 * Determine the pixel size of the new font based on the
	 * point size and resolution.
	 */
	dr = (double) resolution_y;
	dp = (double)(point_size * 10);
	psize = (long)(((dp * dr) / 722.7) + 0.5);

	/*
	 * Make the default width about 1.5 smaller than the height.
	 */
	font->bbx.height = psize;
	font->bbx.width = (unsigned short)((double) psize) / 1.5;

	/*
	 * Now determine the default ascent and descent assuming a
	 * the descent is about 1/4 the ascent.
	 */
	font->bbx.descent = psize >> 2;
	font->bbx.ascent = psize - font->bbx.descent;

	font->bbx.y_offset = -font->bbx.descent;

	/*
	 * Allocation the internal hash tables.
	 */
	font->internal = (void *) malloc(sizeof(hashtable));
	hash_init((hashtable *) font->internal);

	font->default_glyph = -1;
	font->spacing = spacing;

	/*
	 * Add various useful properties.
	 */
	prop.name = "POINT_SIZE";
	prop.format = BDF_INTEGER;
	prop.value.int32 = font->point_size * 10;
	bdf_add_font_property(font, &prop);

	prop.name = "PIXEL_SIZE";
	prop.format = BDF_INTEGER;
	prop.value.int32 = psize;
	bdf_add_font_property(font, &prop);

	prop.name = "RESOLUTION_X";
	prop.format = BDF_CARDINAL;
	prop.value.card32 = (unsigned long) font->resolution_x;
	bdf_add_font_property(font, &prop);

	prop.name = "RESOLUTION_Y";
	prop.format = BDF_CARDINAL;
	prop.value.card32 = (unsigned long) font->resolution_y;
	bdf_add_font_property(font, &prop);

	prop.name = "FONT_ASCENT";
	prop.format = BDF_INTEGER;
	prop.value.int32 = (long) font->bbx.ascent;
	bdf_add_font_property(font, &prop);

	prop.name = "FONT_DESCENT";
	prop.format = BDF_INTEGER;
	prop.value.int32 = (long) font->bbx.descent;
	bdf_add_font_property(font, &prop);

	prop.name = "AVERAGE_WIDTH";
	prop.format = BDF_INTEGER;
	prop.value.int32 = font->bbx.width * 10;
	bdf_add_font_property(font, &prop);

	sp[0] = 'P';
	sp[1] = 0;
	switch (spacing) {
	case BDF_PROPORTIONAL:
		sp[0] = 'P';
		break;
	case BDF_MONOWIDTH:
		sp[0] = 'M';
		break;
	case BDF_CHARCELL:
		sp[0] = 'C';
		break;
	}
	prop.name = "SPACING";
	prop.format = BDF_ATOM;
	prop.value.atom = sp;
	bdf_add_font_property(font, &prop);

	/*
	 * Mark the font as unmodified.
	 */
	font->modified = 0;

	return font;
}

void
#ifdef __STDC__
bdf_set_default_metrics(bdf_font_t *font)
#else
bdf_set_default_metrics(font)
bdf_font_t *font;
#endif
{
	long psize;
	double dp, dr;
	bdf_property_t prop;

	/*
	 * Determine the pixel size of the new font based on the
	 * point size and resolution.
	 */
	dr = (double) font->resolution_y;
	dp = (double)(font->point_size * 10);
	psize = (long)(((dp * dr) / 722.7) + 0.5);

	/*
	 * Make the default width about 1.5 smaller than the height.
	 */
	font->bbx.height = psize;
	font->bbx.width = (unsigned short)((double) psize) / 1.5;

	/*
	 * Now determine the default ascent and descent assuming a
	 * the descent is about 1/4 the ascent.
	 */
	font->bbx.descent = psize >> 2;
	font->bbx.ascent = psize - font->bbx.descent;

	font->bbx.y_offset = -font->bbx.descent;

	font->default_glyph = -1;

	/*
	 * Add various useful properties.
	 */
	prop.name = "FONT_ASCENT";
	prop.format = BDF_INTEGER;
	prop.value.int32 = (long) font->bbx.ascent;
	bdf_add_font_property(font, &prop);

	prop.name = "FONT_DESCENT";
	prop.format = BDF_INTEGER;
	prop.value.int32 = (long) font->bbx.descent;
	bdf_add_font_property(font, &prop);

	prop.name = "AVERAGE_WIDTH";
	prop.format = BDF_INTEGER;
	prop.value.int32 = font->bbx.width * 10;
	bdf_add_font_property(font, &prop);
}

int
#ifdef __STDC__
bdf_glyph_modified(bdf_font_t *font, long which, int unencoded)
#else
bdf_glyph_modified(font, which, unencoded)
bdf_font_t *font;
long which;
int unencoded;
#endif
{
	if (font == 0 || which < 0)
		return 0;

	if (unencoded)
		return _bdf_glyph_modified(font->umod, which);
	else
		return _bdf_glyph_modified(font->nmod, which);
}

void
#ifdef __STDC__
bdf_copy_glyphs(bdf_font_t *font, long start, long end,
                bdf_glyphlist_t *glyphs, int unencoded)
#else
bdf_copy_glyphs(font, start, end, glyphs, unencoded)
bdf_font_t *font;
long start, end;
bdf_glyphlist_t *glyphs;
int unencoded;
#endif
{
	long tmp, i, nc;
	bdf_glyph_t *cp, *dp;
	short maxas, maxds, maxrb, minlb, maxlb, rb;

	if (start > end) {
		tmp = end;
		end = start;
		start = tmp;
	}

	glyphs->bpp = font->bpp;
	glyphs->start = start;
	glyphs->end = end;
	glyphs->glyphs_used = 0;

	tmp = (end - start) + 1;
	if (tmp > glyphs->glyphs_size) {
		if (glyphs->glyphs_size == 0)
			glyphs->glyphs = (bdf_glyph_t *) malloc(sizeof(bdf_glyph_t) * tmp);
		else
			glyphs->glyphs = (bdf_glyph_t *) realloc((char *) glyphs->glyphs,
			                 sizeof(bdf_glyph_t) * tmp);
		cp = glyphs->glyphs + glyphs->glyphs_size;
		(void) memset((char *) cp, 0,
		              sizeof(bdf_glyph_t) * (tmp - glyphs->glyphs_size));
		glyphs->glyphs_size = tmp;
	}

	/*
	 * Clear out bitmaps and names in the existing entries.
	 */
	for (cp = glyphs->glyphs, i = 0; i < glyphs->glyphs_size; i++, cp++) {
		if (cp->name != 0)
			free(cp->name);
		if (cp->bytes > 0)
			free((char *) cp->bitmap);
	}

	/*
	 * Zero out everything.
	 */
	(void) memset((char *) &glyphs->bbx, 0, sizeof(bdf_bbx_t));
	(void) memset((char *) glyphs->glyphs, 0,
	              sizeof(bdf_glyph_t) * glyphs->glyphs_size);

	/*
	 * Initialize the bounds used to generate the overall bounding box for the
	 * set of glyphs being copied.
	 */
	minlb = font->bbx.width;
	maxlb = maxrb = maxas = maxds = 0;

	/*
	 * Do the copy.
	 */
	nc = (unencoded == 0) ? font->glyphs_used : font->unencoded_used;
	cp = (unencoded == 0) ? font->glyphs : font->unencoded;
	dp = glyphs->glyphs;

	for (i = 0;
	        i < nc && ((unencoded && i <= end) || cp->encoding <= end);
	        i++, cp++) {
		if ((unencoded && i >= start) || cp->encoding >= start) {
			(void) memcpy((char *) dp, (char *) cp, sizeof(bdf_glyph_t));
			if (cp->name != 0) {
				dp->name = (char *) malloc(strlen(cp->name) + 1);
				(void) strcpy(dp->name, cp->name);
			}
			if (cp->bytes > 0) {
				dp->bytes = cp->bytes;
				dp->bitmap = (unsigned char *) malloc(cp->bytes);
				(void) memcpy((char *) dp->bitmap, (char *) cp->bitmap,
				              cp->bytes);
			}

			/*
			 * Determine the overall metrics for the group of characters being
			 * copied.
			 */
			maxas = MAX(cp->bbx.ascent, maxas);
			maxds = MAX(cp->bbx.descent, maxds);
			rb = cp->bbx.width + cp->bbx.x_offset;
			maxrb = MAX(rb, maxrb);
			minlb = MIN(cp->bbx.x_offset, minlb);
			maxlb = MAX(cp->bbx.x_offset, maxlb);

			glyphs->glyphs_used++;
			dp++;
		}
	}

	/*
	 * Set the overall metrics for this set of glyphs.
	 */
	glyphs->bbx.width = maxrb - minlb;
	glyphs->bbx.x_offset = minlb;

	glyphs->bbx.height = maxas + maxds;
	glyphs->bbx.ascent = maxas;
	glyphs->bbx.descent = maxds;
	glyphs->bbx.y_offset = -maxds;
}

void
#ifdef __STDC__
bdf_delete_glyphs(bdf_font_t *font, long start, long end, int unencoded)
#else
bdf_delete_glyphs(font, start, end, unencoded)
bdf_font_t *font;
long start, end;
int unencoded;
#endif
{
	long i, n, nc, cnt;
	bdf_glyph_t *cp, *sp, *ep;

	if (font == 0)
		return;

	if (start > end) {
		cnt = end;
		end = start;
		start = cnt;
	}

	nc = (unencoded == 0) ? font->glyphs_used : font->unencoded_used;
	cp = (unencoded == 0) ? font->glyphs : font->unencoded;
	sp = ep = 0;

	for (i = 0; i < nc && cp->encoding <= end; i++, cp++) {
		if (cp->encoding >= start && sp == 0)
			sp = cp;
	}
	ep = cp;
	if (sp == 0)
		sp = ep;

	if (ep > sp) {
		/*
		 * There are some glyphs to delete.
		 * 1. Free the name and bitmap fields of the glyphs being deleted.
		 * 2. Move the end range down if necessary.
		 * 3. Clear the glyphs on the end if a move was done.
		 */

		/*
		 * Mark the font as being modified.
		 */
		font->modified = 1;

		cnt = ep - sp;

		for (cp = sp; cp < ep; cp++) {
			/*
			 * Mark the glyphs being deleted as also being modified so the
			 * empty cells can be shown correctly by the client programs.
			 */
			if (unencoded)
				_bdf_set_glyph_modified(font->umod, cp->encoding);
			else
				_bdf_set_glyph_modified(font->nmod, cp->encoding);

			if (cp->name != 0)
				free(cp->name);
			if (cp->bytes > 0)
				free((char *) cp->bitmap);
		}

		cp = (unencoded == 0) ? font->glyphs : font->unencoded;

		/*
		 * Check to see if there are some glyphs that need to
		 * be moved down.
		 */
		if (ep - cp < nc) {
			/*
			 * Shift the glyphs down.
			 */
			n = nc - (ep - cp);
			_bdf_memmove((char *) sp, (char *) ep, sizeof(bdf_glyph_t) * n);

			/*
			 * Set the starting point for the clear.
			 */
			ep = sp + n;
		} else
			/*
			 * Set the starting point for the clear.
			 */
			ep = sp;

		/*
		 * Clear the glyph space just moved.
		 */
		n = nc - (ep - cp);
		(void) memset((char *) ep, 0, sizeof(bdf_glyph_t) * n);

		/*
		 * Adjust the number of glyphs used.
		 */
		if (unencoded == 0)
			font->glyphs_used -= cnt;
		else
			font->unencoded_used -= cnt;

		/*
		 * If unencoded glyphs were deleted, re-encode all
		 * of them to cause a shift when everything is redrawn.
		 */
		if (unencoded != 0) {
			for (i = 0, cp = font->unencoded; i < font->unencoded_used;
			        i++, cp++) {
				if (_bdf_glyph_modified(font->umod, cp->encoding)) {
					_bdf_clear_glyph_modified(font->umod, cp->encoding);
					_bdf_set_glyph_modified(font->umod, i);
				}
				cp->encoding = i;
			}
		}
	}
}

/*
 * Routines for quick and dirty dithering.
 */
static void
#ifdef __STDC__
_bdf_one_to_n(bdf_glyphlist_t *gl, int n)
#else
_bdf_one_to_n(gl, n)
bdf_glyphlist_t *gl;
int n;
#endif
{
	long i;
	unsigned short bpr, sbpr, bytes, col, sx, sy;
	unsigned char *nbmap, *masks;
	bdf_glyph_t *gp;

	if (gl == 0 || gl->glyphs_used == 0)
		return;

	masks = 0;
	switch (n) {
	case 1:
		masks = onebpp;
		break;
	case 2:
		masks = twobpp;
		break;
	case 4:
		masks = fourbpp;
		break;
	}

	gl->bpp = n;
	for (gp = gl->glyphs, i = 0; i < gl->glyphs_used; i++, gp++) {
		if (gp->bbx.width == 0 || gp->bbx.height == 0)
			continue;
		sbpr = (gp->bbx.width + 7) >> 3;
		bpr = ((gp->bbx.width * n) + 7) >> 3;
		bytes = bpr * gp->bbx.height;
		nbmap = (unsigned char *) malloc(bytes);
		(void) memset((char *) nbmap, 0, bytes);

		for (sy = 0; sy < gp->bbx.height; sy++) {
			for (col = sx = 0; sx < gp->bbx.width; sx++, col += n) {
				if (gp->bitmap[(sy * sbpr) + (sx >> 3)] & (0x80 >> (sx & 7)))
					nbmap[(sy * bpr) + (col >> 3)] |= masks[(col & 7) / n];
			}
		}
		free((char *) gp->bitmap);
		gp->bytes = bytes;
		gp->bitmap = nbmap;
	}
}

static void
#ifdef __STDC__
_bdf_n_to_one(bdf_glyphlist_t *gl)
#else
_bdf_n_to_one(gl)
bdf_glyphlist_t *gl;
#endif
{
	long i;
	unsigned short bpr, sbpr, bytes, col, sx, sy;
	unsigned char *nbmap, *masks;
	bdf_glyph_t *gp;

	if (gl == 0 || gl->glyphs_used == 0)
		return;

	masks = 0;
	switch (gl->bpp) {
	case 1:
		masks = onebpp;
		break;
	case 2:
		masks = twobpp;
		break;
	case 4:
		masks = fourbpp;
		break;
	}

	for (gp = gl->glyphs, i = 0; i < gl->glyphs_used; i++, gp++) {
		if (gp->bbx.width == 0 || gp->bbx.height == 0)
			continue;
		sbpr = ((gp->bbx.width * gl->bpp) + 7) >> 3;
		bpr = (gp->bbx.width + 7) >> 3;
		bytes = bpr * gp->bbx.height;
		nbmap = (unsigned char *) malloc(bytes);
		(void) memset((char *) nbmap, 0, bytes);

		for (sy = 0; sy < gp->bbx.height; sy++) {
			for (col = sx = 0; sx < gp->bbx.width; sx++, col += gl->bpp) {
				if (gp->bitmap[(sy * sbpr) + (col >> 3)] &
				        masks[(col & 7) / gl->bpp])
					nbmap[(sy * bpr) + (sx >> 3)] |= (0x80 >> (sx & 7));
			}
		}
		free((char *) gp->bitmap);
		gp->bytes = bytes;
		gp->bitmap = nbmap;
	}
	gl->bpp = 1;
}

static void
#ifdef __STDC__
_bdf_two_to_four(bdf_glyphlist_t *gl)
#else
_bdf_two_to_four(gl)
bdf_glyphlist_t *gl;
#endif
{
	long i;
	unsigned short bpr, sbpr, bytes, col, si, byte, sx, sy;
	unsigned char *nbmap, *masks;
	bdf_glyph_t *gp;

	if (gl == 0 || gl->glyphs_used == 0)
		return;

	masks = twobpp;

	for (gp = gl->glyphs, i = 0; i < gl->glyphs_used; i++, gp++) {
		if (gp->bbx.width == 0 || gp->bbx.height == 0)
			continue;
		sbpr = ((gp->bbx.width << 1) + 7) >> 3;
		bpr = ((gp->bbx.width << 2) + 7) >> 3;
		bytes = bpr * gp->bbx.height;
		nbmap = (unsigned char *) malloc(bytes);
		(void) memset((char *) nbmap, 0, bytes);

		for (sy = 0; sy < gp->bbx.height; sy++) {
			for (col = sx = 0; sx < gp->bbx.width; sx++, col += 2) {
				si = (col & 7) >> 1;
				byte = gp->bitmap[(sy * sbpr) + (col >> 3)] & masks[si];
				if (byte) {
					/*
					 * Shift the byte down to make an index.
					 */
					if (si < 3)
						byte >>= (3 - si) * gl->bpp;

					/*
					 * Scale the index to four bits per pixel and shift it into
					 * place before adding it.
					 */
					byte = (byte << 2) + 3;
					if ((sx & 1) == 0)
						byte <<= 4;
					nbmap[(sy * bpr) + ((sx << 2) >> 3)] |= byte;
				}
			}
		}
		free((char *) gp->bitmap);
		gp->bytes = bytes;
		gp->bitmap = nbmap;
	}
	gl->bpp = 4;
}

static void
#ifdef __STDC__
_bdf_four_to_two(bdf_glyphlist_t *gl)
#else
_bdf_four_to_two(gl)
bdf_glyphlist_t *gl;
#endif
{
	long i;
	unsigned short bpr, sbpr, bytes, col, si, byte, sx, sy;
	unsigned char *nbmap, *masks;
	bdf_glyph_t *gp;

	if (gl == 0 || gl->glyphs_used == 0)
		return;

	masks = fourbpp;

	gl->bpp = 2;
	for (gp = gl->glyphs, i = 0; i < gl->glyphs_used; i++, gp++) {
		sbpr = ((gp->bbx.width << 2) + 7) >> 3;
		bpr = ((gp->bbx.width << 1) + 7) >> 3;
		bytes = bpr * gp->bbx.height;
		nbmap = (unsigned char *) malloc(bytes);
		(void) memset((char *) nbmap, 0, bytes);

		for (sy = 0; sy < gp->bbx.height; sy++) {
			for (col = sx = 0; sx < gp->bbx.width; sx++, col += 4) {
				si = (col & 7) >> 2;
				byte = gp->bitmap[(sy * sbpr) + (col >> 3)] & masks[si];
				if (byte) {
					/*
					 * Shift the byte down to make an index.
					 */
					if (si == 0)
						byte >>= 4;

					/*
					 * Scale the index to two bits per pixel and shift it into
					 * place if necessary.
					 */
					byte >>= 2;

					si = ((sx << 1) & 7) >> 1;
					if (si < 3)
						byte <<= (3 - si) << 1;

					nbmap[(sy * bpr) + ((sx << 1) >> 3)] |= byte;
				}
			}
		}
		free((char *) gp->bitmap);
		gp->bytes = bytes;
		gp->bitmap = nbmap;
	}
}

int
#ifdef __STDC__
bdf_replace_glyphs(bdf_font_t *font, long start, bdf_glyphlist_t *glyphs,
                   int unencoded)
#else
bdf_replace_glyphs(font, start, glyphs, unencoded)
bdf_font_t *font;
long start;
bdf_glyphlist_t *glyphs;
int unencoded;
#endif
{
	int resize, appending;
	long i, n, ng, end, del, remaining, off[2];
	bdf_glyph_t *sgp, *gp, *dgp;
	short maxas, maxds, maxrb, minlb, maxlb, rb;
	double ps, rx, dw;
	bdf_bbx_t nbbx;

	resize = 0;

	if (font == 0)
		return resize;

	/*
	 * Dither the incoming bitmaps so they match the same bits per pixel as
	 * the font.
	 */
	if (glyphs->bpp != font->bpp) {
		if (glyphs->bpp == 1)
			_bdf_one_to_n(glyphs, font->bpp);
		else if (font->bpp == 1)
			_bdf_n_to_one(glyphs);
		else if (glyphs->bpp == 2)
			_bdf_two_to_four(glyphs);
		else
			_bdf_four_to_two(glyphs);
	}

	/*
	 * Set the point size and horizontal resolution so the scalable width can
	 * be determined.
	 */
	ps = (double) font->point_size;
	rx = (double) font->resolution_x;

	/*
	 * Determine if a resize is needed.
	 */

	/*
	 * Determine the bounding box for the font without the characters being
	 * replaced.
	 */
	minlb = 32767;
	maxlb = maxrb = maxas = maxds = 0;

	/*
	 * Get the font bounds.
	 */
	maxas = MAX(font->bbx.ascent, maxas);
	maxds = MAX(font->bbx.descent, maxds);
	rb = font->bbx.width + font->bbx.x_offset;
	maxrb = MAX(rb, maxrb);
	minlb = MIN(font->bbx.x_offset, minlb);
	maxlb = MAX(font->bbx.x_offset, maxlb);

	/*
	 * Get the bounds of the incoming glyphs.
	 */
	maxas = MAX(glyphs->bbx.ascent, maxas);
	maxds = MAX(glyphs->bbx.descent, maxds);
	rb = glyphs->bbx.width + glyphs->bbx.x_offset;
	maxrb = MAX(rb, maxrb);
	minlb = MIN(glyphs->bbx.x_offset, minlb);
	maxlb = MAX(glyphs->bbx.x_offset, maxlb);

	/*
	 * Set up the new font bounding box, minus the characters that are being
	 * removed and with the new characters added.
	 */
	nbbx.width = maxrb - minlb;
	nbbx.x_offset = minlb;

	nbbx.height = maxas + maxds;
	nbbx.ascent = maxas;
	nbbx.descent = maxds;
	nbbx.y_offset = -maxds;

	/*
	 * Now determine if the combination of the glyphs removed and the new
	 * glyphs cause the font bounding box to be changed.
	 */
	resize = (nbbx.width > font->bbx.width ||
	          nbbx.height > font->bbx.height) ? 1 : 0;

	/*
	 * Set the pointers to the glyphs.
	 */
	ng = (unencoded == 0) ? font->glyphs_used : font->unencoded_used;
	sgp = gp = (unencoded == 0) ? font->glyphs : font->unencoded;

	/*
	 * Locate the closest glyph on or following `start'.
	 */
	for (i = 0; i < ng && gp->encoding < start; i++, gp++) ;

	appending = (i == ng);

	/*
	 * Set the starting point for copying the incoming glyphs.
	 */
	dgp = gp;

	n = glyphs->end - glyphs->start;
	end = start + n;

	/*
	 * Delete all the glyphs between `start' and `end'.
	 */
	for (del = 0, i = start; i <= end; i++) {
		/*
		 * Mark the character as being modified.
		 */
		if (ng > 0 && !appending && gp->encoding == i) {
			if (unencoded == 0)
				_bdf_set_glyph_modified(font->nmod, i);
			else
				_bdf_set_glyph_modified(font->umod, i);

			if (gp->name != 0)
				free(gp->name);
			if (gp->bytes > 0)
				free((char *) gp->bitmap);
			del++;
			gp++;
		}
	}

	/*
	 * Determine how many glyphs remain following the last one deleted.
	 */
	remaining = ng - (gp - sgp);

	if (glyphs->glyphs_used == 0) {
		/*
		 * If the glyph list is empty, then shift any remaining glyphs down
		 * to the destination.
		 */
		_bdf_memmove((char *) dgp, (char *) gp,
		             sizeof(bdf_glyph_t) * remaining);
		if (unencoded == 0)
			font->glyphs_used -= del;
		else
			font->unencoded_used -= del;
	} else {
		/*
		 * Insert the glyph list after making sure there is enough space to
		 * hold them.  Also adjust the encoding and scalable width values
		 * after copying the glyphs.
		 */
		if (unencoded == 0) {
			n = (font->glyphs_used - del) + glyphs->glyphs_used;
			if (n > font->glyphs_size) {
				off[0] = gp - sgp;
				off[1] = dgp - sgp;
				if (font->glyphs_size == 0)
					font->glyphs = sgp =
					                   (bdf_glyph_t *) malloc(sizeof(bdf_glyph_t) * n);
				else
					font->glyphs = sgp =
					                   (bdf_glyph_t *) realloc((char *) font->glyphs,
					                                           sizeof(bdf_glyph_t) * n);
				gp = sgp + off[0];
				dgp = sgp + off[1];
				font->glyphs_size = n;
			}

			/*
			 * Calculate how many will need to be shifted.
			 */
			if ((n = glyphs->glyphs_used - del) >= font->glyphs_used)
				n = 0;
		} else {
			n = (font->unencoded_used - del) + glyphs->glyphs_used;
			if (n > font->unencoded_size) {
				off[0] = gp - sgp;
				off[1] = dgp - sgp;
				if (font->unencoded_size == 0)
					font->unencoded = sgp =
					                      (bdf_glyph_t *) malloc(sizeof(bdf_glyph_t) * n);
				else
					font->unencoded = sgp =
					                      (bdf_glyph_t *) realloc((char *) font->unencoded,
					                                              sizeof(bdf_glyph_t) * n);
				gp = sgp + off[0];
				dgp = sgp + off[1];
				font->unencoded_size = n;
			}

			/*
			 * Calculate how many will need to be shifted.
			 */
			if ((n = glyphs->glyphs_used - del) >= font->unencoded_used)
				n = 0;
		}

		/*
		 * Shift any following glyphs up or down if needed.
		 */
		if (n)
			_bdf_memmove((char *)(gp + n), (char *) gp,
			             sizeof(bdf_glyph_t) * remaining);

		/*
		 * Copy the incoming glyphs, copy their names and bitmaps,
		 * set their encodings, and set their scalable widths.
		 */
		(void) memcpy((char *) dgp, (char *) glyphs->glyphs,
		              sizeof(bdf_glyph_t) * glyphs->glyphs_used);
		for (i = 0; i < glyphs->glyphs_used; i++, dgp++) {
			if (dgp->name != 0)
				dgp->name = (char *) _bdf_strdup((unsigned char *) dgp->name,
				                                 strlen(dgp->name) + 1);

			if (dgp->bytes > 0)
				dgp->bitmap = _bdf_strdup(dgp->bitmap, dgp->bytes);

			dgp->encoding = start + (dgp->encoding - glyphs->start);

			/*
			 * Mark the glyph as being modified in case it fills a cell that
			 * was empty before.
			 */
			_bdf_set_glyph_modified(font->nmod, dgp->encoding);

			dw = (double) dgp->dwidth;
			dgp->swidth = (unsigned short)((dw * 72000.0) / (ps * rx));
		}

		/*
		 * Adjust the count of glyphs.
		 */
		ng = (ng - del) + glyphs->glyphs_used;
		if (unencoded == 0)
			font->glyphs_used = ng;
		else
			font->unencoded_used = ng;
	}

	/*
	 * Last, if the replacement was done in the unencoded section,
	 * reencode all the glyphs so they show up properly.
	 */
	if (unencoded != 0) {
		for (i = 0; i < ng; i++, sgp++) {
			if (_bdf_glyph_modified(font->umod, sgp->encoding)) {
				_bdf_clear_glyph_modified(font->umod, sgp->encoding);
				_bdf_set_glyph_modified(font->umod, i);
			}
			sgp->encoding = i;
		}
	}

	if (resize)
		(void) memcpy((char *) &font->bbx, (char *) &nbbx, sizeof(bdf_bbx_t));

	font->modified = 1;

	return resize;
}

int
#ifdef __STDC__
bdf_insert_glyphs(bdf_font_t *font, long start, bdf_glyphlist_t *glyphs)
#else
bdf_insert_glyphs(font, start, glyphs)
bdf_font_t *font;
long start;
bdf_glyphlist_t *glyphs;
#endif
{
	int resize;
	unsigned long i, ng, n, which;
	bdf_glyph_t *gp;

	resize = 0;

	if (font == 0)
		return resize;

	/*
	 * Dither the incoming bitmaps so they match the same bits per pixel as
	 * the font.
	 */
	if (glyphs->bpp != font->bpp) {
		if (glyphs->bpp == 1)
			_bdf_one_to_n(glyphs, font->bpp);
		else if (font->bpp == 1)
			_bdf_n_to_one(glyphs);
		else if (glyphs->bpp == 2)
			_bdf_two_to_four(glyphs);
		else
			_bdf_four_to_two(glyphs);
	}

	/*
	 * Locate the starting glyph.
	 */
	gp = font->glyphs;
	ng = font->glyphs_used;
	for (i = 0; i < ng && gp->encoding < start; i++, gp++) ;

	/*
	 * If there are no glyphs at the starting point, then simply do a replace.
	 */
	if (i == ng)
		return bdf_replace_glyphs(font, start, glyphs, 0);

	/*
	 * Go through the glyphs that would be shifted due to the insertion and
	 * determine if some of them will overflow the 0xffff boundary.
	 */
	n = (glyphs->end - glyphs->start) + 1;
	for (which = i; i < ng; i++, gp++) {
		if (gp->encoding + n > 0xffff)
			break;
	}

	if (i < ng) {
		/*
		 * Some glyphs have to be moved to the unencoded area because they
		 * would overflow the 0xffff boundary if they were moved up.
		 */
		bdf_copy_glyphs(font, gp->encoding, font->glyphs[ng - 1].encoding,
		                &font->overflow, 0);
		bdf_delete_glyphs(font,  gp->encoding, font->glyphs[ng - 1].encoding,
		                  0);
		resize += bdf_replace_glyphs(font, font->unencoded_used,
		                             &font->overflow, 1);
	}

	/*
	 * Go back to the insertion point and shift the remaining glyph encodings
	 * up by `n'.
	 */
	for (gp = font->glyphs + which; which < font->glyphs_used; which++, gp++) {
		/*
		 * Mark the new glyph locations as being modified.
		 */
		gp->encoding += n;
		_bdf_set_glyph_modified(font->nmod, gp->encoding);
	}

	/*
	 * Finally, mark the font as being modified and insert the new glyphs.
	 */
	font->modified = 1;

	return resize + bdf_replace_glyphs(font, start, glyphs, 0);
}

static void
#ifdef __STDC__
_bdf_combine_glyphs(bdf_font_t *font, bdf_glyph_t *f, bdf_glyph_t *g)
#else
_bdf_combine_glyphs(font, f, g)
bdf_font_t *font;
bdf_glyph_t *f, *g;
#endif
{
	unsigned short x, sx, sy, si, dx, dy, di, byte, dbpr, fbpr, gbpr;
	short maxas, maxds, maxrb, minlb, maxlb, rb;
	unsigned char *masks;
	bdf_bbx_t nbbx;
	bdf_glyph_t tmp;

	/*
	 * Determine the max bounding box for the two glyphs.
	 */
	minlb = 32767;
	maxlb = maxrb = maxas = maxds = 0;

	/*
	 * Get the font glyph bounds.
	 */
	maxas = MAX(f->bbx.ascent, maxas);
	maxds = MAX(f->bbx.descent, maxds);
	rb = f->bbx.width + f->bbx.x_offset;
	maxrb = MAX(rb, maxrb);
	minlb = MIN(f->bbx.x_offset, minlb);
	maxlb = MAX(f->bbx.x_offset, maxlb);

	/*
	 * Get the bounds of the incoming glyph.
	 */
	maxas = MAX(g->bbx.ascent, maxas);
	maxds = MAX(g->bbx.descent, maxds);
	rb = g->bbx.width + g->bbx.x_offset;
	maxrb = MAX(rb, maxrb);
	minlb = MIN(g->bbx.x_offset, minlb);
	maxlb = MAX(g->bbx.x_offset, maxlb);

	/*
	 * Set up the new glyph bounding box.
	 */
	nbbx.width = maxrb - minlb;
	nbbx.x_offset = minlb;
	nbbx.height = maxas + maxds;
	nbbx.ascent = maxas;
	nbbx.descent = maxds;
	nbbx.y_offset = -maxds;

	masks = 0;
	switch (font->bpp) {
	case 1:
		masks = onebpp;
		break;
	case 2:
		masks = twobpp;
		break;
	case 4:
		masks = fourbpp;
		break;
	}

	fbpr = ((f->bbx.width * font->bpp) + 7) >> 3;
	gbpr = ((g->bbx.width * font->bpp) + 7) >> 3;
	dbpr = ((nbbx.width * font->bpp) + 7) >> 3;

	if (memcmp((char *) &nbbx, (char *) &f->bbx, sizeof(bdf_bbx_t)) == 0) {
		/*
		 * The largest is the first, so merge the second in with it.
		 */
		dy = f->bbx.ascent - g->bbx.ascent;
		for (sy = 0; sy < g->bbx.height; sy++, dy++) {
			for (x = sx = 0; x < g->bbx.width; x++, sx += font->bpp) {
				si = (sx & 7) / font->bpp;
				if ((byte = g->bitmap[(sy * gbpr) + (sx >> 3)] & masks[si]))
					/*
					 * No shifting of the byte is needed because the x offset
					 * is the same for both glyphs.
					 */
					f->bitmap[(dy * fbpr) + (sx >> 3)] |= byte;
			}
		}
	} else if (memcmp((char *) &nbbx, (char *) &g->bbx,
	                  sizeof(bdf_bbx_t)) == 0) {
		/*
		 * The largest is the incoming glyph, so merge into that one and swap
		 * it with the font glyph.
		 */
		dy = g->bbx.ascent - f->bbx.ascent;
		for (sy = 0; sy < f->bbx.height; sy++, dy++) {
			for (x = sx = 0; x < f->bbx.width; x++, sx += font->bpp) {
				si = (sx & 7) / font->bpp;
				if ((byte = f->bitmap[(sy * gbpr) + (sx >> 3)] & masks[si]))
					/*
					 * No shifting of the byte is needed because the x offset
					 * is the same for both glyphs.
					 */
					g->bitmap[(dy * fbpr) + (sx >> 3)] |= byte;
			}
		}

		/*
		 * Now swap the two glyphs while preserving the name and encoding of
		 * the first glyph.
		 */
		tmp.swidth = g->swidth;
		tmp.dwidth = g->dwidth;
		tmp.bytes = g->bytes;
		tmp.bitmap = g->bitmap;
		(void) memcpy((char *) &tmp.bbx, (char *) &g->bbx, sizeof(bdf_bbx_t));

		g->swidth = f->swidth;
		g->dwidth = f->dwidth;
		g->bytes = f->bytes;
		g->bitmap = f->bitmap;
		(void) memcpy((char *) &g->bbx, (char *) &f->bbx, sizeof(bdf_bbx_t));

		f->swidth = tmp.swidth;
		f->dwidth = tmp.dwidth;
		f->bytes = tmp.bytes;
		f->bitmap = tmp.bitmap;
		(void) memcpy((char *) &f->bbx, (char *) &tmp.bbx, sizeof(bdf_bbx_t));
	} else {
		/*
		 * Need a new bitmap for the combination of the two.
		 */
		tmp.bytes = nbbx.height * dbpr;
		tmp.bitmap = (unsigned char *) malloc(tmp.bytes);
		(void) memset((char *) tmp.bitmap, 0, tmp.bytes);

		/*
		 * Merge the first glyph.
		 */
		dy = nbbx.ascent - f->bbx.ascent;
		for (sy = 0; sy < f->bbx.height; sy++, dy++) {
			dx = MYABS(nbbx.x_offset - f->bbx.x_offset) * font->bpp;
			for (x = sx = 0; x < f->bbx.width; x++,
			        sx += font->bpp, dx += font->bpp) {
				si = (sx & 7) / font->bpp;
				if ((byte = f->bitmap[(sy * fbpr) + (sx >> 3)] & masks[si])) {
					di = (dx & 7) / font->bpp;
					if (di < si)
						byte <<= (si - di) * font->bpp;
					else if (di > si)
						byte >>= (di - si) * font->bpp;
					tmp.bitmap[(dy * dbpr) + (dx >> 3)] |= byte;
				}
			}
		}

		/*
		 * Merge the second glyph.
		 */
		dy = nbbx.ascent - g->bbx.ascent;
		for (sy = 0; sy < g->bbx.height; sy++, dy++) {
			dx = MYABS(nbbx.x_offset - g->bbx.x_offset) * font->bpp;
			for (x = sx = 0; x < g->bbx.width; x++,
			        sx += font->bpp, dx += font->bpp) {
				si = (sx & 7) / font->bpp;
				if ((byte = g->bitmap[(sy * gbpr) + (sx >> 3)] & masks[si])) {
					di = (dx & 7) / font->bpp;
					if (di < si)
						byte <<= (si - di) * font->bpp;
					else if (di > si)
						byte >>= (di - si) * font->bpp;
					tmp.bitmap[(dy * dbpr) + (dx >> 3)] |= byte;
				}
			}
		}

		/*
		 * Now clear the font glyph and copy the temp glyph to it.
		 */
		if (f->bytes > 0)
			free((char *) f->bitmap);
		f->bytes = tmp.bytes;
		f->bitmap = tmp.bitmap;
		(void) memcpy((char *) &f->bbx, (char *) &nbbx, sizeof(bdf_bbx_t));

		/*
		 * Set the device width.  Pay attention to whether the font is
		 * monowidth or character cell.
		 */
		if (font->spacing != BDF_PROPORTIONAL)
			f->dwidth = font->monowidth;
		else
			f->dwidth = MAX(f->dwidth, g->dwidth);
	}
}

int
#ifdef __STDC__
bdf_merge_glyphs(bdf_font_t *font, long start, bdf_glyphlist_t *glyphs,
                 int unencoded)
#else
bdf_merge_glyphs(font, start, glyphs, unencoded)
bdf_font_t *font;
long start;
bdf_glyphlist_t *glyphs;
int unencoded;
#endif
{
	int resize;
	long i, n, ng, end, add, enc, off;
	bdf_glyph_t *sgp, *gp, *dgp, *base;
	short maxas, maxds, maxrb, minlb, maxlb, rb;
	double ps, rx, dw;
	bdf_bbx_t nbbx;

	resize = 0;

	if (font == 0)
		return resize;

	/*
	 * If the glyphs are being merged in the unencoded area, simply append
	 * them.  The unencoded area is simply storage.
	 */
	if (unencoded)
		return bdf_replace_glyphs(font, font->unencoded_used, glyphs, unencoded);

	/*
	 * Dither the incoming bitmaps so they match the same bits per pixel as
	 * the font.
	 */
	if (glyphs->bpp != font->bpp) {
		if (glyphs->bpp == 1)
			_bdf_one_to_n(glyphs, font->bpp);
		else if (font->bpp == 1)
			_bdf_n_to_one(glyphs);
		else if (glyphs->bpp == 2)
			_bdf_two_to_four(glyphs);
		else
			_bdf_four_to_two(glyphs);
	}

	/*
	 * Set the point size and horizontal resolution so the scalable width can
	 * be determined.
	 */
	ps = (double) font->point_size;
	rx = (double) font->resolution_x;

	/*
	 * Determine if a resize is needed.
	 */

	/*
	 * Determine the bounding box for the font without the characters being
	 * replaced.
	 */
	minlb = 32767;
	maxlb = maxrb = maxas = maxds = 0;

	/*
	 * Get the font bounds.
	 */
	maxas = MAX(font->bbx.ascent, maxas);
	maxds = MAX(font->bbx.descent, maxds);
	rb = font->bbx.width + font->bbx.x_offset;
	maxrb = MAX(rb, maxrb);
	minlb = MIN(font->bbx.x_offset, minlb);
	maxlb = MAX(font->bbx.x_offset, maxlb);

	/*
	 * Get the bounds of the incoming glyphs.
	 */
	maxas = MAX(glyphs->bbx.ascent, maxas);
	maxds = MAX(glyphs->bbx.descent, maxds);
	rb = glyphs->bbx.width + glyphs->bbx.x_offset;
	maxrb = MAX(rb, maxrb);
	minlb = MIN(glyphs->bbx.x_offset, minlb);
	maxlb = MAX(glyphs->bbx.x_offset, maxlb);

	/*
	 * Set up the new font bounding box, minus the characters that are being
	 * removed and with the new characters added.
	 */
	nbbx.width = maxrb - minlb;
	nbbx.x_offset = minlb;

	nbbx.height = maxas + maxds;
	nbbx.ascent = maxas;
	nbbx.descent = maxds;
	nbbx.y_offset = -maxds;

	/*
	 * Now determine if the combination of the glyphs removed and the new
	 * glyphs cause the font bounding box to be changed.
	 */
	resize = (nbbx.width > font->bbx.width ||
	          nbbx.height > font->bbx.height) ? 1 : 0;

	/*
	 * Set the pointers to the glyphs.
	 */
	ng = (unencoded == 0) ? font->glyphs_used : font->unencoded_used;
	gp = (unencoded == 0) ? font->glyphs : font->unencoded;

	/*
	 * Locate the closest glyph on or following `start'.
	 */
	for (i = 0; i < ng && gp->encoding < start; i++, gp++) ;

	if (i == ng)
		/*
		 * If the gylphs are being added off the end of the list, simply insert
		 * them so any overflows can be handled.
		 */
		return bdf_insert_glyphs(font, start, glyphs);

	/*
	 * Set the starting point for copying the incoming glyphs.
	 */
	dgp = gp;

	n = glyphs->end - glyphs->start;
	end = start + n;

	/*
	 * Count the number of glyphs that will be added and mark all the
	 * glyphs that will be modified.
	 */
	for (sgp = glyphs->glyphs, add = 0, i = start; i <= end; i++) {
		enc = (sgp->encoding - glyphs->start) + start;

		/*
		 * Mark the glyph as being modified.
		 */
		if (unencoded == 0)
			_bdf_set_glyph_modified(font->nmod, enc);
		else
			_bdf_set_glyph_modified(font->umod, enc);

		if (enc == gp->encoding)
			sgp++;
		else if (enc < gp->encoding) {
			add++;
			sgp++;
		}

		if (gp->encoding == i)
			gp++;
	}

	if (add > 0) {
		ng += add;

		/*
		 * Need to make room for some glyphs that will be added.
		 */
		if (unencoded) {
			off = dgp - font->unencoded;
			if (font->unencoded_used == 0)
				font->unencoded =
				    (bdf_glyph_t *) malloc(sizeof(bdf_glyph_t) * ng);
			else
				font->unencoded =
				    (bdf_glyph_t *) realloc((char *) font->unencoded,
				                            sizeof(bdf_glyph_t) * ng);
			dgp = font->unencoded + off;
			font->unencoded_used = ng;
		} else {
			off = dgp - font->glyphs;
			if (font->glyphs_used == 0)
				font->glyphs =
				    (bdf_glyph_t *) malloc(sizeof(bdf_glyph_t) * ng);
			else
				font->glyphs =
				    (bdf_glyph_t *) realloc((char *) font->glyphs,
				                            sizeof(bdf_glyph_t) * ng);
			dgp = font->glyphs + off;
			font->glyphs_used = ng;
		}
	}

	/*
	 * Now go through and do two things:
	 * 1. Insert new incoming glyphs.
	 * 2. Combine two glyphs at the same location.
	 */
	base = (!unencoded) ? font->glyphs : font->unencoded;
	for (gp = dgp, sgp = glyphs->glyphs, i = start; i <= end; i++) {
		enc = (sgp->encoding - glyphs->start) + start;
		if (enc < gp->encoding) {
			/*
			 * Shift the glyphs up by one and add this one.
			 */
			if (gp - base < ng)
				_bdf_memmove((char *)(gp + 1), (char *) gp,
				             sizeof(bdf_glyph_t) * (ng - (gp - base)));
			(void) memcpy((char *) gp, (char *) sgp, sizeof(bdf_glyph_t));
			gp->name = (char *) _bdf_strdup((unsigned char *) gp->name,
			                                strlen(gp->name) + 1);
			if (gp->bytes > 0)
				gp->bitmap = _bdf_strdup(gp->bitmap, gp->bytes);
			gp->encoding = i;
			sgp++;
			dw = (double) gp->dwidth;
			gp->swidth = (unsigned short)((dw * 72000.0) / (ps * rx));
		} else if (enc == gp->encoding) {
			_bdf_combine_glyphs(font, gp, sgp);
			dw = (double) gp->dwidth;
			gp->swidth = (unsigned short)((dw * 72000.0) / (ps * rx));
			sgp++;
		}
		if (gp->encoding == i)
			gp++;
	}

	if (resize)
		(void) memcpy((char *) &font->bbx, (char *) &nbbx, sizeof(bdf_bbx_t));

	font->modified = 1;

	return resize;
}

void
#ifdef __STDC__
bdf_set_modified(bdf_font_t *font, int modified)
#else
bdf_set_modified(font, modified)
bdf_font_t *font;
int modified;
#endif
{
	if (font == 0 || font->modified == modified)
		return;

	if (modified == 0) {
		/*
		 * Clear out the modified bitmaps.
		 */
		(void) memset((char *) font->nmod, 0, sizeof(unsigned long) * 2048);
		(void) memset((char *) font->umod, 0, sizeof(unsigned long) * 2048);
	}
	font->modified = modified;
}

/**************************************************************************
 *
 * XLFD font name functions.
 *
 **************************************************************************/

static char *xlfdfields[] = {
	"FOUNDRY",
	"FAMILY_NAME",
	"WEIGHT_NAME",
	"SLANT",
	"SETWIDTH_NAME",
	"ADD_STYLE_NAME",
	"PIXEL_SIZE",
	"POINT_SIZE",
	"RESOLUTION_X",
	"RESOLUTION_Y",
	"SPACING",
	"AVERAGE_WIDTH",
	"CHARSET_REGISTRY",
	"CHARSET_ENCODING",
};

int
#ifdef __STDC__
bdf_has_xlfd_name(bdf_font_t *font)
#else
bdf_has_xlfd_name(font)
bdf_font_t *font;
#endif
{
	unsigned long len;
	char name[256];
	_bdf_list_t list;

	if (font == 0 || font->name == 0 || font->name[0] == 0)
		return 0;

	len = (unsigned long)(strlen(font->name) + 1);
	(void) memcpy(name, font->name, len);
	list.size = list.used = 0;
	_bdf_split("-", name, len, &list);
	if (list.size > 0)
		free((char *) list.field);

	return (list.used == 15);
}

char *
#ifdef __STDC__
bdf_make_xlfd_name(bdf_font_t *font, char *foundry, char *family)
#else
bdf_make_xlfd_name(font, foundry, family)
bdf_font_t *font;
char *foundry, *family;
#endif
{
	int len;
	double dp, dr;
	unsigned long i, width, used;
	unsigned short awidth, pxsize;
	bdf_property_t *pp;
	bdf_glyph_t *gp;
	char spacing, nbuf[256], *np, *name;
	const char *val;

	if (font == 0 || bdf_has_xlfd_name(font))
		return 0;

	np = nbuf;

	/*
	 * Add the FOUNDRY field.
	 */
	if ((pp = bdf_get_font_property(font, "FOUNDRY")) != 0)
		foundry = pp->value.atom;
	sprintf(np, "-%s", foundry);
	np += strlen(np);

	/*
	 * Add the FAMILY_NAME field.
	 */
	if ((pp = bdf_get_font_property(font, "FAMILY_NAME")) != 0)
		family = pp->value.atom;
	sprintf(np, "-%s", family);
	np += strlen(np);

	/*
	 * Add the WEIGHT_NAME field.
	 */
	val = ((pp = bdf_get_font_property(font, "WEIGHT_NAME")) != 0) ?
	      pp->value.atom : "Medium";
	if (val == 0)
		val = "Medium";
	sprintf(np, "-%s", val);
	np += strlen(np);

	/*
	 * Add the SLANT field.
	 */
	val = ((pp = bdf_get_font_property(font, "SLANT")) != 0) ?
	      pp->value.atom : "R";
	if (val == 0)
		val = "R";
	sprintf(np, "-%s", val);
	np += strlen(np);

	/*
	 * Add the SETWIDTH_NAME field.
	 */
	val = ((pp = bdf_get_font_property(font, "SETWIDTH_NAME")) != 0) ?
	      pp->value.atom : "Normal";
	if (val == 0)
		val = "Normal";
	sprintf(np, "-%s", val);
	np += strlen(np);

	/*
	 * Add the ADD_STYLE_NAME field.
	 */
	val = ((pp = bdf_get_font_property(font, "ADD_STYLE_NAME")) != 0) ?
	      pp->value.atom : "";
	if (val == 0)
		val = "";
	sprintf(np, "-%s", val);
	np += strlen(np);

	/*
	 * Add the PIXEL_SIZE field.
	 */
	if ((pp = bdf_get_font_property(font, "PIXEL_SIZE")) != 0)
		sprintf(np, "-%ld", pp->value.int32);
	else {
		/*
		 * Determine the pixel size.
		 */
		dp = (double)(font->point_size * 10);
		dr = (double) font->resolution_y;
		pxsize = (unsigned short)(((dp * dr) / 722.7) + 0.5);
		sprintf(np, "-%hd", pxsize);
	}
	np += strlen(np);

	/*
	 * Add the POINT_SIZE field.
	 */
	if ((pp = bdf_get_font_property(font, "POINT_SIZE")) != 0)
		sprintf(np, "-%ld", pp->value.int32);
	else
		sprintf(np, "-%ld", font->point_size * 10);
	np += strlen(np);

	/*
	 * Add the RESOLUTION_X field.
	 */
	if ((pp = bdf_get_font_property(font, "RESOLUTION_X")) != 0)
		sprintf(np, "-%ld", pp->value.card32);
	else
		sprintf(np, "-%ld", font->resolution_x);
	np += strlen(np);

	/*
	 * Add the RESOLUTION_Y field.
	 */
	if ((pp = bdf_get_font_property(font, "RESOLUTION_Y")) != 0)
		sprintf(np, "-%ld", pp->value.card32);
	else
		sprintf(np, "-%ld", font->resolution_y);
	np += strlen(np);

	/*
	 * Add the SPACING field.
	 */
	if ((pp = bdf_get_font_property(font, "SPACING")) != 0)
		spacing = pp->value.atom[0];
	else {
		spacing = 'P';
		switch (font->spacing) {
		case BDF_PROPORTIONAL:
			spacing = 'P';
			break;
		case BDF_MONOWIDTH:
			spacing = 'M';
			break;
		case BDF_CHARCELL:
			spacing = 'C';
			break;
		}
	}
	sprintf(np, "-%c", spacing);
	np += strlen(np);

	/*
	 * Add the AVERAGE_WIDTH field.
	 */
	if ((pp = bdf_get_font_property(font, "AVERAGE_WIDTH")) != 0)
		sprintf(np, "-%ld", pp->value.int32);
	else {
		/*
		 * Determine the average width of all the glyphs in the font.
		 */
		width = 0;
		for (i = 0, gp = font->unencoded; i < font->unencoded_used; i++, gp++)
			width += gp->dwidth;
		for (i = 0, gp = font->glyphs; i < font->glyphs_used; i++, gp++)
			width += gp->dwidth;

		used = font->unencoded_used + font->glyphs_used;
		if (used == 0)
			awidth = font->bbx.width * 10;
		else
			awidth = (unsigned short)((((float) width) /
			                           ((float) used)) * 10.0);
		sprintf(np, "-%hd", awidth);
	}
	np += strlen(np);

	/*
	 * Add the CHARSET_REGISTRY field.
	 */
	val = ((pp = bdf_get_font_property(font, "CHARSET_REGISTRY")) != 0) ?
	      pp->value.atom : "FontSpecific";
	sprintf(np, "-%s", val);
	np += strlen(np);

	/*
	 * Add the CHARSET_ENCODING field.
	 */
	val = ((pp = bdf_get_font_property(font, "CHARSET_ENCODING")) != 0) ?
	      pp->value.atom : "0";
	sprintf(np, "-%s", val);
	np += strlen(np);

	len = (np - nbuf) + 1;
	name = (char *) malloc(len);
	(void) memcpy(name, nbuf, len);
	return name;
}

void
#ifdef __STDC__
bdf_update_name_from_properties(bdf_font_t *font)
#else
bdf_update_name_from_properties(font)
bdf_font_t *font;
#endif
{
	unsigned long i;
	bdf_property_t *p;
	_bdf_list_t list;
	char *np, name[128], nname[128];

	if (font == 0 || bdf_has_xlfd_name(font) == 0)
		return;

	(void) memset((char *) &list, 0, sizeof(_bdf_list_t));

	/*
	 * Split the name into fields and shift out the first empty field.
	 * This assumes that the font has a name.
	 */
	i = (unsigned long) strlen(font->name);
	(void) memcpy(name, font->name, i + 1);
	_bdf_split("-", name, i, &list);
	_bdf_shift(1, &list);

	/*
	 * Initialize the pointer to the new name and add the '-' prefix.
	 */
	np = nname;
	*np++ = '-';
	*np = 0;

	for (i = 0; i < 14; i++) {
		if ((p = bdf_get_font_property(font, xlfdfields[i])) != 0) {
			/*
			 * The property exists, so add it to the new font name.
			 */
			switch (p->format) {
			case BDF_ATOM:
				if (p->value.atom != 0)
					sprintf(np, "%s", p->value.atom);
				break;
			case BDF_CARDINAL:
				sprintf(np, "%ld", p->value.card32);
				break;
			case BDF_INTEGER:
				sprintf(np, "%ld", p->value.int32);
				break;
			}
		} else
			/*
			 * The property does not exist, so add the original value to the
			 * new font name.
			 */
			sprintf(np, "%s", list.field[i]);
		np += strlen(np);
		if (i + 1 < 14) {
			*np++ = '-';
			*np = 0;
		}
	}

	/*
	 * Replace the existing font name with the new one.
	 */
	free(font->name);
	i = (unsigned long)(strlen(nname) + 1);
	font->name = (char *) malloc(i);
	(void) memcpy(font->name, nname, i);

	/*
	 * Free up the list.
	 */
	if (list.size > 0)
		free((char *) list.field);

	font->modified = 1;
}

void
#ifdef __STDC__
bdf_update_properties_from_name(bdf_font_t *font)
#else
bdf_update_properties_from_name(font)
bdf_font_t *font;
#endif
{
	unsigned long i;
	bdf_property_t *p, prop;
	_bdf_list_t list;
	char name[128];

	if (font == 0 || font->name == 0 || bdf_has_xlfd_name(font) == 0)
		return;

	(void) memset((char *) &list, 0, sizeof(_bdf_list_t));

	/*
	 * Split the name into fields and shift out the first empty field.
	 */
	i = (unsigned long) strlen(font->name);
	(void) memcpy(name, font->name, i + 1);
	_bdf_split("-", name, i, &list);
	_bdf_shift(1, &list);

	for (i = 0; i < 14; i++) {
		p = bdf_get_property(xlfdfields[i]);
		prop.name = p->name;
		prop.format = p->format;
		switch (prop.format) {
		case BDF_ATOM:
			prop.value.atom = list.field[i];
			break;
		case BDF_CARDINAL:
			prop.value.card32 = _bdf_atoul(list.field[i], 0, 10);
			break;
		case BDF_INTEGER:
			prop.value.int32 = _bdf_atol(list.field[i], 0, 10);
			break;
		}
		bdf_add_font_property(font, &prop);
	}

	/*
	 * Free up the list.
	 */
	if (list.size > 0)
		free((char *) list.field);

	font->modified = 1;
}

int
#ifdef __STDC__
bdf_update_average_width(bdf_font_t *font)
#else
bdf_update_average_width(font)
bdf_font_t *font;
#endif
{
	int changed;
	unsigned long i;
	long oaw, awidth, used;
	bdf_glyph_t *gp;
	_bdf_list_t list;
	bdf_property_t *pp, prop;
	char *np, num[16], nbuf[128];

	changed = 0;

	used = font->unencoded_used + font->glyphs_used;
	if (used == 0)
		awidth = font->bbx.width * 10;
	else {
		for (i = 0, awidth = 0, gp = font->unencoded; i < font->unencoded_used;
		        i++, gp++)
			awidth += gp->dwidth;
		for (i = 0, gp = font->glyphs; i < font->glyphs_used; i++, gp++)
			awidth += gp->dwidth;
		awidth = (long)((((double) awidth) / ((double) used)) * 10.0);
	}

	/*
	 * Check to see if it is different than the average width in the font
	 * name.
	 */
	if (bdf_has_xlfd_name(font)) {
		(void) memset((char *) &list, 0, sizeof(_bdf_list_t));
		i = (unsigned long) strlen(font->name);
		(void) memcpy(nbuf, font->name, i + 1);
		_bdf_split("-", nbuf, i, &list);
		oaw = _bdf_atol(list.field[12], 0, 10);
		if (oaw != awidth) {
			/*
			 * Construct a new font name with the new average width.
			 */
			changed = 1;
			sprintf(num, "%ld", awidth);
			used = strlen(num) - strlen(list.field[12]);
			if (used > 0) {
				/*
				 * Resize the string used for the font name instead of
				 * creating a new one.
				 */
				used += i;
				font->name = (char *) realloc(font->name, used);
			}

			/*
			 * Copy the elements of the list back into the new font name.
			 */
			np = font->name;
			*np++ = '-';
			for (i = 1; i < list.used; i++) {
				if (i == 12)
					strcpy(np, num);
				else
					strcpy(np, list.field[i]);
				np += strlen(np);
				if (i + 1 < list.used)
					*np++ = '-';
			}
		}

		/*
		 * Clear up any space allocated for the list.
		 */
		if (list.size > 0)
			free((char *) list.field);
	}

	/*
	 * Now check for the AVERAGE_WIDTH property.
	 */
	if ((pp = bdf_get_font_property(font, "AVERAGE_WIDTH")) != 0) {
		if (pp->value.int32 != awidth) {
			changed = 1;
			pp->value.int32 = awidth;
		}
	} else {
		/*
		 * Property doesn't exist yet, so add it.
		 */
		changed = 1;
		prop.name = "AVERAGE_WIDTH";
		prop.format = BDF_INTEGER;
		prop.value.int32 = awidth;
		bdf_add_font_property(font, &prop);
	}

	if (changed)
		font->modified = 1;

	return changed;
}

/*
 * Change the font bounding box and return a non-zero number if this causes
 * the font to get larger or smaller.
 */
int
#ifdef __STDC__
bdf_set_font_bbx(bdf_font_t *font, bdf_metrics_t *metrics)
#else
bdf_set_font_bbx(font, metrics)
bdf_font_t *font;
bdf_metrics_t *metrics;
#endif
{
	int resize;

	resize = 0;

	if (font == 0 || metrics == 0)
		return resize;

	resize = (font->bbx.width != metrics->width ||
	          font->bbx.height != metrics->height) ? 1 : 0;

	font->bbx.width = metrics->width;
	font->bbx.height = metrics->height;
	font->bbx.x_offset = metrics->x_offset;
	font->bbx.y_offset = metrics->y_offset;
	font->bbx.ascent = metrics->ascent;
	font->bbx.descent = metrics->descent;

	/*
	 * If the font is not proportional, then make sure the monowidth field is
	 * set to the font bounding box.
	 */
	if (font->spacing != BDF_PROPORTIONAL)
		font->monowidth = font->bbx.width;

	return resize;
}

static bdf_glyph_t *
#ifdef __STDC__
_bdf_locate_glyph(bdf_font_t *font, long code, int unencoded)
#else
_bdf_locate_glyph(font, code, unencoded)
bdf_font_t *font;
long code;
int unencoded;
#endif
{
	long l, r, m, nc;
	bdf_glyph_t *gl;

	if (code < 0 || font == 0)
		return 0;

	if ((unencoded && font->unencoded_used == 0) ||
	        font->glyphs_used == 0)
		return 0;

	if (unencoded) {
		gl = font->unencoded;
		nc = font->unencoded_used;
	} else {
		gl = font->glyphs;
		nc = font->glyphs_used;
	}
	for (l = m = 0, r = nc - 1; l < r;) {
		m = (l + r) >> 1;
		if (gl[m].encoding < code)
			l = m + 1;
		else if (gl[m].encoding > code)
			r = m - 1;
		else
			break;
	}

	/*
	 * Go back until we hit the beginning of the glyphs or until
	 * we find the glyph with a code less than the specified code.
	 */
	while (m > 0 && gl[m].encoding > code)
		m--;

	/*
	 * Look forward if necessary.
	 */
	while (m < nc && gl[m].encoding < code)
		m++;

	return (m < nc) ? &gl[m] : &gl[nc - 1];
}

int
#ifdef __STDC__
bdf_translate_glyphs(bdf_font_t *font, short dx, short dy, long start,
                     long end, bdf_callback_t callback, void *data,
                     int unencoded)
#else
bdf_translate_glyphs(font, dx, dy, start, end, callback, data, unencoded)
bdf_font_t *font;
short dx, dy;
long start, end;
bdf_callback_t callback;
void *data;
int unencoded;
#endif
{
	int resize, diff;
	bdf_glyph_t *gp, *sp, *ep;
	bdf_callback_struct_t cb;

	if (font == 0 || (dx == 0 && dy == 0))
		return 0;

	if ((unencoded && font->unencoded_used == 0) || font->glyphs_used == 0)
		return 0;

	/*
	 * Call the progress initialization callback.
	 */
	if (callback != 0) {
		cb.reason = BDF_TRANSLATE_START;
		cb.total = (end - start) + 1;
		cb.current = 0;
		(*callback)(&cb, data);
	}

	/*
	 * Locate the first and last glyphs to be shifted.
	 */
	sp = _bdf_locate_glyph(font, start, unencoded);
	ep = _bdf_locate_glyph(font, end, unencoded);
	for (resize = 0, gp = sp; sp <= ep; sp++) {
		/*
		 * Call the callback if one was provided.
		 */
		if (sp != gp && callback != 0) {
			cb.reason = BDF_TRANSLATING;
			cb.current = (sp->encoding - start) + 1;
			(*callback)(&cb, data);
		}

		/*
		 * Apply the X translation.
		 */
		if (dx != 0) {
			sp->bbx.x_offset += dx;
			diff = sp->bbx.x_offset - font->bbx.x_offset;
			if (sp->bbx.x_offset < font->bbx.x_offset) {
				font->bbx.x_offset = sp->bbx.x_offset;
				font->bbx.width += MYABS(diff);
				resize = 1;
			} else if (sp->bbx.width + sp->bbx.x_offset >
			           font->bbx.width + font->bbx.x_offset) {
				font->bbx.width += MYABS(diff);
				resize = 1;
			}

			/*
			 * Mark the glyph as modified appropriately.
			 */
			if (unencoded)
				_bdf_set_glyph_modified(font->umod, sp->encoding);
			else
				_bdf_set_glyph_modified(font->nmod, sp->encoding);
		}

		/*
		 * Apply the Y translation.
		 */
		if (dy != 0) {
			sp->bbx.y_offset += dy;
			sp->bbx.descent = -sp->bbx.y_offset;
			sp->bbx.ascent = sp->bbx.height - sp->bbx.descent;
			diff = sp->bbx.y_offset - font->bbx.y_offset;
			if (sp->bbx.y_offset < font->bbx.y_offset) {
				font->bbx.y_offset = sp->bbx.y_offset;
				font->bbx.descent = -font->bbx.y_offset;
				font->bbx.height += MYABS(diff);
				resize = 1;
			} else if (sp->bbx.ascent > font->bbx.ascent) {
				font->bbx.ascent += MYABS(diff);
				font->bbx.height += MYABS(diff);
				resize = 1;
			}

			/*
			 * Mark the glyph as modified appropriately.
			 */
			if (unencoded)
				_bdf_set_glyph_modified(font->umod, sp->encoding);
			else
				_bdf_set_glyph_modified(font->nmod, sp->encoding);
		}
	}

	/*
	 * Call the callback one more time to make sure the client knows
	 * this is done.
	 */
	if (callback != 0 && cb.current < cb.total) {
		cb.reason = BDF_TRANSLATING;
		cb.current = cb.total;
		(*callback)(&cb, data);
	}

	if (resize)
		font->modified = 1;

	return resize;
}

static void
#ifdef __STDC__
_bdf_resize_rotation(bdf_font_t *font, int mul90, short degrees,
                     bdf_glyph_t *glyph, bdf_bitmap_t *scratch,
                     unsigned short *width, unsigned short *height)
#else
_bdf_resize_rotation(font, mul90, degrees, glyph, scratch, width, height)
bdf_font_t *font;
int mul90;
short degrees;
bdf_glyph_t *glyph;
bdf_bitmap_t *scratch;
unsigned short *width, *height;
#endif
{
	unsigned short w, h, wd, ht, bytes;
	short cx, cy, x1, y1, x2, y2;
	double dx1, dy1, dx2, dy2;

	w = h = 0;

	cx = glyph->bbx.width >> 1;
	cy = glyph->bbx.height >> 1;

	/*
	 * Rotate the lower left and upper right corners and check for a potential
	 * resize.
	 */
	x1 = 0;
	y1 = glyph->bbx.height;
	x2 = glyph->bbx.width;
	y2 = 0;

	dx1 = (double)(x1 - cx);
	dy1 = (double)(y1 - cy);
	dx2 = (double)(x2 - cx);
	dy2 = (double)(y2 - cx);

	if (mul90) {
		x1 = cx + (short)((dx1 * _bdf_cos_tbl[degrees]) -
		                  (dy1 * _bdf_sin_tbl[degrees]));
		y1 = cy + (short)((dx1 * _bdf_sin_tbl[degrees]) +
		                  (dy1 * _bdf_cos_tbl[degrees]));
		x2 = cx + (short)((dx2 * _bdf_cos_tbl[degrees]) -
		                  (dy2 * _bdf_sin_tbl[degrees]));
		y2 = cy + (short)((dx2 * _bdf_sin_tbl[degrees]) +
		                  (dy2 * _bdf_cos_tbl[degrees]));
	} else {
		x1 = cx + _bdf_ceiling((dx1 * _bdf_cos_tbl[degrees]) -
		                       (dy1 * _bdf_sin_tbl[degrees]));
		y1 = cy + _bdf_ceiling((dx1 * _bdf_sin_tbl[degrees]) +
		                       (dy1 * _bdf_cos_tbl[degrees]));
		x2 = cx + _bdf_ceiling((dx2 * _bdf_cos_tbl[degrees]) -
		                       (dy2 * _bdf_sin_tbl[degrees]));
		y2 = cy + _bdf_ceiling((dx2 * _bdf_sin_tbl[degrees]) +
		                       (dy2 * _bdf_cos_tbl[degrees]));
	}

	wd = MYABS(x2 - x1);
	ht = MYABS(y2 - y1);

	w = MAX(wd, w);
	h = MAX(ht, h);

	if (wd > font->bbx.width)
		font->bbx.width += wd - font->bbx.width;
	if (ht > font->bbx.height) {
		font->bbx.ascent += ht - font->bbx.height;
		font->bbx.height += ht - font->bbx.height;
	}

	/*
	 * Rotate the upper left and lower right corners and check for a potential
	 * resize.
	 */
	x1 = 0;
	y1 = 0;
	x2 = glyph->bbx.width;
	y2 = glyph->bbx.height;

	dx1 = (double)(x1 - cx);
	dy1 = (double)(y1 - cy);
	dx2 = (double)(x2 - cx);
	dy2 = (double)(y2 - cx);

	if (mul90) {
		x1 = cx + (short)((dx1 * _bdf_cos_tbl[degrees]) -
		                  (dy1 * _bdf_sin_tbl[degrees]));
		y1 = cy + (short)((dx1 * _bdf_sin_tbl[degrees]) +
		                  (dy1 * _bdf_cos_tbl[degrees]));
		x2 = cx + (short)((dx2 * _bdf_cos_tbl[degrees]) -
		                  (dy2 * _bdf_sin_tbl[degrees]));
		y2 = cy + (short)((dx2 * _bdf_sin_tbl[degrees]) +
		                  (dy2 * _bdf_cos_tbl[degrees]));
	} else {
		x1 = cx + _bdf_ceiling((dx1 * _bdf_cos_tbl[degrees]) -
		                       (dy1 * _bdf_sin_tbl[degrees]));
		y1 = cy + _bdf_ceiling((dx1 * _bdf_sin_tbl[degrees]) +
		                       (dy1 * _bdf_cos_tbl[degrees]));
		x2 = cx + _bdf_ceiling((dx2 * _bdf_cos_tbl[degrees]) -
		                       (dy2 * _bdf_sin_tbl[degrees]));
		y2 = cy + _bdf_ceiling((dx2 * _bdf_sin_tbl[degrees]) +
		                       (dy2 * _bdf_cos_tbl[degrees]));
	}

	wd = MYABS(x2 - x1);
	ht = MYABS(y2 - y1);

	w = MAX(wd, w);
	h = MAX(ht, h);

	if (wd > font->bbx.width)
		font->bbx.width += wd - font->bbx.width;
	if (ht > font->bbx.height) {
		font->bbx.ascent += ht - font->bbx.height;
		font->bbx.height += ht - font->bbx.height;
	}

	if (font->bbx.width > scratch->width ||
	        font->bbx.height > scratch->height) {
		scratch->width = MAX(font->bbx.width, scratch->width);
		scratch->height = MAX(font->bbx.height, scratch->height);
		bytes = (((font->bbx.width * font->bpp) + 7) >> 3) * font->bbx.height;
		if (scratch->bytes == 0)
			scratch->bitmap = (unsigned char *) malloc(bytes);
		else
			scratch->bitmap = (unsigned char *)
			                  realloc((char *) scratch->bitmap, bytes);
		scratch->bytes = bytes;
	}

	/*
	 * Clear the bitmap.
	 */
	(void) memset((char *) scratch->bitmap, 0, scratch->bytes);

	/*
	 * Return the new glyph width and height.
	 */
	*width = w;
	*height = h;
}

int
#ifdef __STDC__
bdf_rotate_glyphs(bdf_font_t *font, short degrees, long start,
                  long end, bdf_callback_t callback, void *data,
                  int unencoded)
#else
bdf_rotate_glyphs(font, degrees, start, end, callback, data, unencoded)
bdf_font_t *font;
short degrees;
long start, end;
bdf_callback_t callback;
void *data;
int unencoded;
#endif
{
	int mul90, bpr, sbpr;
	unsigned short wd, ht, si, di, byte, col;
	short x, y, cx, cy, nx, ny, ox, oy, shiftx, shifty;
	bdf_glyph_t *gp, *sp, *ep;
	unsigned char *masks;
	double dx, dy;
	bdf_bitmap_t scratch;
	bdf_callback_struct_t cb;

	if (font == 0 || (unencoded && font->unencoded_used == 0) ||
	        font->glyphs_used == 0)
		return 0;

	while (degrees < 0)
		degrees += 360;
	while (degrees >= 360)
		degrees -= 360;

	if (degrees == 0)
		return 0;

	mul90 = ((degrees % 90) == 0) ? 1 : 0;

	masks = 0;
	switch (font->bpp) {
	case 1:
		masks = onebpp;
		break;
	case 2:
		masks = twobpp;
		break;
	case 4:
		masks = fourbpp;
		break;
	}

	/*
	 * Initialize the scratch bitmap.
	 */
	(void) memset((char *) &scratch, 0, sizeof(bdf_bitmap_t));

	/*
	 * Call the progress initialization callback.
	 */
	if (callback != 0) {
		cb.reason = BDF_ROTATE_START;
		cb.total = (end - start) + 1;
		cb.current = 0;
		(*callback)(&cb, data);
	}

	sp = _bdf_locate_glyph(font, start, unencoded);
	ep = _bdf_locate_glyph(font, end, unencoded);
	for (gp = sp; sp <= ep; sp++) {
		/*
		 * Call the callback if one was provided.
		 */
		if (sp != gp && callback != 0) {
			cb.reason = BDF_ROTATING;
			cb.current = (sp->encoding - start) + 1;
			(*callback)(&cb, data);
		}

		/*
		 * Resize the bitmap, adjust the font bounding box, and get the new
		 * glyph width and height.
		 */
		_bdf_resize_rotation(font, mul90, degrees, sp, &scratch, &wd, &ht);

		cx = sp->bbx.width >> 1;
		cy = sp->bbx.height >> 1;

		shiftx = shifty = 0;
		sbpr = ((wd * font->bpp) + 7) >> 3;
		bpr = ((sp->bbx.width * font->bpp) + 7) >> 3;
		for (y = 0; y < sp->bbx.height; y++) {
			for (col = x = 0; x < sp->bbx.width; x++, col += font->bpp) {
				si = (col & 7) / font->bpp;
				byte = sp->bitmap[(y * bpr) + (col >> 3)] & masks[si];
				if (byte) {
					dx = (double)(x - cx);
					dy = (double)(y - cy);
					if (mul90) {
						nx = cx + (short)((dx * _bdf_cos_tbl[degrees]) -
						                  (dy * _bdf_sin_tbl[degrees]));
						ny = cy + (short)((dx * _bdf_sin_tbl[degrees]) +
						                  (dy * _bdf_cos_tbl[degrees]));
					} else {
						nx = cx + _bdf_ceiling((dx * _bdf_cos_tbl[degrees]) -
						                       (dy * _bdf_sin_tbl[degrees]));
						ny = cy + _bdf_ceiling((dx * _bdf_sin_tbl[degrees]) +
						                       (dy * _bdf_cos_tbl[degrees]));
					}
					if (nx < 0) {
						shiftx = MIN(shiftx, nx);
						nx += wd;
					} else if (nx >= wd) {
						ox = (nx - wd) + 1;
						shiftx = MAX(shiftx, ox);
						nx -= wd;
					}
					if (ny < 0) {
						shifty = MIN(shifty, ny);
						ny += ht;
					} else if (ny >= ht) {
						oy = (ny - ht) + 1;
						shifty = MAX(shifty, oy);
						ny -= ht;
					}
					nx *= font->bpp;
					di = (nx & 7) / font->bpp;
					if (di < si)
						byte <<= (si - di) * font->bpp;
					else if (di > si)
						byte >>= (di - si) * font->bpp;
					scratch.bitmap[(ny * sbpr) + (nx >> 3)] |= byte;
				}
			}
		}
		/*
		 * Resize the glyph bitmap if necessary.
		 */
		if (wd != sp->bbx.width || ht != sp->bbx.height) {
			sp->bbx.width = wd;
			sp->bbx.height = ht;
			sp->bbx.ascent = ht - sp->bbx.descent;
			sp->bytes = (((wd * font->bpp) + 7) >> 3) * ht;
			sp->bitmap = (unsigned char *)
			             realloc((char *) sp->bitmap, sp->bytes);
		}
		(void) memset((char *) sp->bitmap, 0, sp->bytes);

		/*
		 * Copy the glyph from the scratch area to the glyph bitmap,
		 * adjusting for any shift values encountered.
		 */
		bpr = ((sp->bbx.width * font->bpp) + 7) >> 3;
		for (y = 0; y < sp->bbx.height; y++) {
			for (col = x = 0; x < sp->bbx.width; x++, col += font->bpp) {
				si = (col & 7) / font->bpp;
				byte = scratch.bitmap[(y * bpr) + (col >> 3)] & masks[si];
				if (byte) {
					nx = x - shiftx;
					ny = y - shifty;
					if (nx < 0)
						nx += sp->bbx.width;
					else if (nx >= sp->bbx.width)
						nx -= sp->bbx.width;
					if (ny < 0)
						ny += sp->bbx.height;
					else if (ny >= sp->bbx.height)
						ny -= sp->bbx.height;
					nx *= font->bpp;
					di = (nx & 7) / font->bpp;
					if (di < si)
						byte <<= (si - di) * font->bpp;
					else if (di > si)
						byte >>= (di - si) * font->bpp;
					sp->bitmap[(ny * bpr) + (nx >> 3)] |= byte;
				}
			}
		}
		/*
		 * Mark the glyph as modified.
		 */
		if (unencoded)
			_bdf_set_glyph_modified(font->umod, sp->encoding);
		else
			_bdf_set_glyph_modified(font->nmod, sp->encoding);
	}

	/*
	 * Call the callback one more time to make sure the client knows
	 * this is done.
	 */
	if (callback != 0 && cb.current < cb.total) {
		cb.reason = BDF_TRANSLATING;
		cb.current = cb.total;
		(*callback)(&cb, data);
	}

	if (scratch.bytes > 0)
		free((char *) scratch.bitmap);

	/*
	 * Rotations always change things, so just return a value indicating this.
	 */
	font->modified = 1;
	return 1;
}

static void
#ifdef __STDC__
_bdf_resize_shear(bdf_font_t *font, int neg, short degrees,
                  bdf_glyph_t *glyph, bdf_bitmap_t *scratch,
                  unsigned short *width, unsigned short *height)
#else
_bdf_resize_shear(font, neg, degrees, glyph, scratch, width, height)
bdf_font_t *font;
int neg;
short degrees;
bdf_glyph_t *glyph;
bdf_bitmap_t *scratch;
unsigned short *width, *height;
#endif
{
	unsigned short wd, w, bytes;
	short x1, y1, x2, y2;

	w = 0;
	*height = glyph->bbx.height;

	/*
	 * Shear the lower left and upper right corners and check for a potential
	 * resize.
	 */
	x1 = 0;
	y1 = glyph->bbx.height;
	x2 = glyph->bbx.width;
	y2 = 0;

	if (neg) {
		x1 += (short)((double) y1 * _bdf_tan_tbl[degrees]);
		x2 += (short)((double) y2 * _bdf_tan_tbl[degrees]);
	} else {
		x1 += (short)((double)(glyph->bbx.height - y1) *
		              _bdf_tan_tbl[degrees]);
		x2 += (short)((double)(glyph->bbx.height - y2) *
		              _bdf_tan_tbl[degrees]);
	}

	wd = MYABS(x2 - x1);
	w = MAX(w, wd);

	if (wd > font->bbx.width)
		font->bbx.width += wd - font->bbx.width;

	/*
	 * Shear the upper left and lower right corners and check for a potential
	 * resize.
	 */
	x1 = 0;
	y1 = 0;
	x2 = glyph->bbx.width;
	y2 = glyph->bbx.height;

	if (neg) {
		x1 += (short)((double) y1 * _bdf_tan_tbl[degrees]);
		x2 += (short)((double) y2 * _bdf_tan_tbl[degrees]);
	} else {
		x1 += (short)((double)(glyph->bbx.height - y1) *
		              _bdf_tan_tbl[degrees]);
		x2 += (short)((double)(glyph->bbx.height - y2) *
		              _bdf_tan_tbl[degrees]);
	}

	wd = MYABS(x2 - x1);
	w = MAX(w, wd);

	if (wd > font->bbx.width)
		font->bbx.width += wd - font->bbx.width;

	if (font->bbx.width > scratch->width ||
	        font->bbx.height > scratch->height) {
		scratch->width = MAX(font->bbx.width, scratch->width);
		scratch->height = MAX(font->bbx.height, scratch->height);
		bytes = (((font->bbx.width * font->bpp) + 7) >> 3) * font->bbx.height;
		if (scratch->bytes == 0)
			scratch->bitmap = (unsigned char *) malloc(bytes);
		else
			scratch->bitmap = (unsigned char *)
			                  realloc((char *) scratch->bitmap, bytes);
		scratch->bytes = bytes;
	}

	/*
	 * Clear the bitmap.
	 */
	(void) memset((char *) scratch->bitmap, 0, scratch->bytes);

	/*
	 * Return the new glyph width.
	 */
	*width = w;
}

int
#ifdef __STDC__
bdf_shear_glyphs(bdf_font_t *font, short degrees, long start,
                 long end, bdf_callback_t callback, void *data,
                 int unencoded)
#else
bdf_shear_glyphs(font, degrees, start, end, callback, data, unencoded)
bdf_font_t *font;
short degrees;
long start, end;
bdf_callback_t callback;
void *data;
int unencoded;
#endif
{
	int neg, bpr, sbpr;
	unsigned short wd, ht, si, di, byte, col;
	short x, y, nx, shiftx, ox;
	bdf_glyph_t *gp, *sp, *ep;
	unsigned char *masks;
	bdf_bitmap_t scratch;
	bdf_callback_struct_t cb;

	if (font == 0 || (unencoded && font->unencoded_used == 0) ||
	        font->glyphs_used == 0)
		return 0;

	if (degrees == 0 || degrees < -45 || degrees > 45)
		return 0;

	if ((neg = (degrees < 0)))
		degrees = -degrees;

	masks = 0;
	switch (font->bpp) {
	case 1:
		masks = onebpp;
		break;
	case 2:
		masks = twobpp;
		break;
	case 4:
		masks = fourbpp;
		break;
	}

	/*
	 * Initialize the scratch bitmap.
	 */
	(void) memset((char *) &scratch, 0, sizeof(bdf_bitmap_t));

	/*
	 * Call the progress initialization callback.
	 */
	if (callback != 0) {
		cb.reason = BDF_SHEAR_START;
		cb.total = (end - start) + 1;
		cb.current = 0;
		(*callback)(&cb, data);
	}

	sp = _bdf_locate_glyph(font, start, unencoded);
	ep = _bdf_locate_glyph(font, end, unencoded);
	for (gp = sp; sp <= ep; sp++) {
		/*
		 * Call the callback if one was provided.
		 */
		if (sp != gp && callback != 0) {
			cb.reason = BDF_SHEARING;
			cb.current = (sp->encoding - start) + 1;
			(*callback)(&cb, data);
		}

		/*
		 * Resize the bitmap, adjust the font bounding box, and get the new
		 * glyph width and height.
		 */
		_bdf_resize_shear(font, neg, degrees, sp, &scratch, &wd, &ht);

		shiftx = 0;
		sbpr = ((wd * font->bpp) + 7) >> 3;
		bpr = ((sp->bbx.width * font->bpp) + 7) >> 3;
		for (y = 0; y < sp->bbx.height; y++) {
			for (col = x = 0; x < sp->bbx.width; x++, col += font->bpp) {
				si = (col & 7) / font->bpp;
				byte = sp->bitmap[(y * bpr) + (col >> 3)] & masks[si];
				if (byte) {
					if (neg)
						nx = x + (short)((double) y * _bdf_tan_tbl[degrees]);
					else
						nx = x + (short)((double)(sp->bbx.height - y) *
						                 _bdf_tan_tbl[degrees]);

					if (nx < 0) {
						shiftx = MIN(shiftx, nx);
						nx += wd;
					} else if (nx >= wd) {
						ox = (nx - wd) + 1;
						shiftx = MAX(shiftx, ox);
						nx -= wd;
					}
					nx *= font->bpp;
					di = (nx & 7) / font->bpp;
					if (di < si)
						byte <<= (si - di) * font->bpp;
					else if (di > si)
						byte >>= (di - si) * font->bpp;
					scratch.bitmap[(y * sbpr) + (nx >> 3)] |= byte;
				}
			}
		}
		/*
		 * Resize the glyph bitmap if necessary.
		 */
		if (wd != sp->bbx.width || ht != sp->bbx.height) {
			sp->bbx.width = wd;
			sp->bbx.height = ht;
			sp->bbx.ascent = ht - sp->bbx.descent;
			sp->bytes = (((wd * font->bpp) + 7) >> 3) * ht;
			sp->bitmap = (unsigned char *)
			             realloc((char *) sp->bitmap, sp->bytes);
		}
		(void) memset((char *) sp->bitmap, 0, sp->bytes);

		/*
		 * Copy the glyph from the scratch area to the glyph bitmap,
		 * adjusting for any shift values encountered.
		 */
		bpr = ((sp->bbx.width * font->bpp) + 7) >> 3;
		for (y = 0; y < sp->bbx.height; y++) {
			for (col = x = 0; x < sp->bbx.width; x++, col += font->bpp) {
				si = (col & 7) / font->bpp;
				byte = scratch.bitmap[(y * bpr) + (col >> 3)] & masks[si];
				if (byte) {
					nx = x - shiftx;
					if (nx < 0)
						nx += sp->bbx.width;
					else if (nx >= sp->bbx.width)
						nx -= sp->bbx.width;
					nx *= font->bpp;
					di = (nx & 7) / font->bpp;
					if (di < si)
						byte <<= (si - di) * font->bpp;
					else if (di > si)
						byte >>= (di - si) * font->bpp;
					sp->bitmap[(y * bpr) + (nx >> 3)] |= byte;
				}
			}
		}
		/*
		 * Mark the glyph as modified.
		 */
		if (unencoded)
			_bdf_set_glyph_modified(font->umod, sp->encoding);
		else
			_bdf_set_glyph_modified(font->nmod, sp->encoding);
	}

	/*
	 * Call the callback one more time to make sure the client knows
	 * this is done.
	 */
	if (callback != 0 && cb.current < cb.total) {
		cb.reason = BDF_TRANSLATING;
		cb.current = cb.total;
		(*callback)(&cb, data);
	}

	if (scratch.bytes > 0)
		free((char *) scratch.bitmap);

	/*
	 * Rotations always change things, so just return a value indicating this.
	 */
	font->modified = 1;
	return 1;
}

static void
#ifdef __STDC__
_bdf_widen_by(bdf_font_t *f, bdf_glyph_t *g, bdf_bitmap_t *s, int n)
#else
_bdf_widen_by(f, g, s, n)
bdf_font_t *f;
bdf_glyph_t *g;
bdf_bitmap_t *s;
int n;
#endif
{
	int bytes, sbpr, dbpr, col;
	short x, y, si, di;
	unsigned char *bmap, *masks;

	masks = 0;
	switch (f->bpp) {
	case 1:
		masks = onebpp;
		break;
	case 2:
		masks = twobpp;
		break;
	case 4:
		masks = fourbpp;
		break;
	}

	s->height = g->bbx.height;
	s->width = g->bbx.width + n;

	bytes = (((s->width * f->bpp) + 7) >> 3) * s->height;

	if (s->bytes == 0)
		s->bitmap = (unsigned char *) malloc(bytes);
	else
		s->bitmap = (unsigned char *)
		            realloc((char *) s->bitmap, bytes);
	s->bytes = bytes;

	(void) memset((char *) s->bitmap, 0, s->bytes);

	/*
	 * Copy the glyph bitmap to the scratch area, and then swap the bitmaps.
	 */
	sbpr = ((g->bbx.width * f->bpp) + 7) >> 3;
	dbpr = ((s->width * f->bpp) + 7) >> 3;
	for (y = 0; y < g->bbx.height; y++) {
		for (col = x = 0; x < g->bbx.width; x++, col += f->bpp) {
			si = (col & 7) / f->bpp;
			bytes = g->bitmap[(y * sbpr) + (col >> 3)] & masks[si];
			if (bytes) {
				di = ((x * f->bpp) & 7) / f->bpp;
				if (di < si)
					bytes <<= (si - di) * f->bpp;
				else if (di > si)
					bytes >>= (di - si) * f->bpp;
				s->bitmap[(y * dbpr) + (col >> 3)] |= bytes;
			}
		}
	}
	g->bbx.width = s->width;

	/*
	 * Swap the bytes and bitmap fields from the scratch area and the glyph.
	 */
	bytes = g->bytes;
	g->bytes = s->bytes;
	s->bytes = bytes;

	bmap = g->bitmap;
	g->bitmap = s->bitmap;
	s->bitmap = bmap;
}

int
#ifdef __STDC__
bdf_embolden_glyphs(bdf_font_t *font, long start, long end,
                    bdf_callback_t callback, void *data, int unencoded,
                    int *resize)
#else
bdf_embolden_glyphs(font, start, end, callback, data, unencoded, resize)
bdf_font_t *font;
long start, end;
bdf_callback_t callback;
void *data;
int unencoded, *resize;
#endif
{
	int mod, gmod, bpr;
	short x, y;
	unsigned short si, di, b1, b2, col;
	unsigned char *masks;
	bdf_glyph_t *gp, *sp, *ep;
	bdf_bitmap_t scratch;
	bdf_callback_struct_t cb;

	if (font == 0 || (unencoded && font->unencoded_used == 0) ||
	        font->glyphs_used == 0)
		return 0;

	/*
	 * Initialize the scratch bitmap which may be needed.
	 */
	(void) memset((char *) &scratch, 0, sizeof(bdf_bitmap_t));

	mod = 0;
	gp = 0;

	masks = 0;
	switch (font->bpp) {
	case 1:
		masks = onebpp;
		break;
	case 2:
		masks = twobpp;
		break;
	case 4:
		masks = fourbpp;
		break;
	}

	/*
	 * Call the progress initialization callback.
	 */
	if (callback != 0) {
		cb.reason = BDF_EMBOLDEN_START;
		cb.total = (end - start) + 1;
		cb.current = 0;
		(*callback)(&cb, data);
	}

	/*
	 * Initialize the resize flag for the caller.
	 */
	*resize = 0;

	sp = _bdf_locate_glyph(font, start, unencoded);
	ep = _bdf_locate_glyph(font, end, unencoded);
	for (; sp <= ep; sp++) {
		/*
		 * Call the callback if one was provided.
		 */
		if (sp != gp && callback != 0) {
			cb.reason = BDF_EMBOLDENING;
			cb.current = (sp->encoding - start) + 1;
			(*callback)(&cb, data);
		}

		if (font->spacing == BDF_PROPORTIONAL ||
		        (font->spacing == BDF_MONOWIDTH &&
		         sp->bbx.width < font->bbx.width)) {
			/*
			 * Only widen the glyph if it is within reason.
			 */
			_bdf_widen_by(font, sp, &scratch, 1);

			if (sp->bbx.width > font->bbx.width) {
				/*
				 * Bump the font width up by the difference.
				 */
				font->bbx.width += sp->bbx.width - font->bbx.width;
				*resize = 1;
			}
		}

		gmod = 0;
		bpr = ((sp->bbx.width * font->bpp) + 7) >> 3;
		for (y = 0; y < sp->bbx.height; y++) {
			col = (sp->bbx.width - 1) * font->bpp;
			for (x = sp->bbx.width - 1; x > 0; x--, col -= font->bpp) {
				si = (col & 7) / font->bpp;
				di = ((col - font->bpp) & 7) / font->bpp;
				b1 = (x == sp->bbx.width) ? 0 :
				     sp->bitmap[(y * bpr) + (col >> 3)] & masks[si];
				b2 = sp->bitmap[(y * bpr) + ((col - font->bpp) >> 3)] &
				     masks[di];
				if (!b1 && b2) {
					if (di < si)
						b2 >>= (si - di) * font->bpp;
					else if (di > si)
						b2 <<= (di - si) * font->bpp;
					sp->bitmap[(y * bpr) + (col >> 3)] |= b2;
					gmod = mod = 1;
				}
			}
		}
		/*
		 * Mark the glyph as modified.
		 */
		if (gmod) {
			if (unencoded)
				_bdf_set_glyph_modified(font->umod, sp->encoding);
			else
				_bdf_set_glyph_modified(font->nmod, sp->encoding);
		}
	}

	/*
	 * Call the callback one more time to make sure the client knows
	 * this is done.
	 */
	if (callback != 0 && cb.current < cb.total) {
		cb.reason = BDF_EMBOLDENING;
		cb.current = cb.total;
		(*callback)(&cb, data);
	}

	/*
	 * Deallocate the scratch bitmap if necessary.
	 */
	if (scratch.bytes > 0)
		free((char *) scratch.bitmap);

	font->modified = mod;

	return mod;
}

static int _endian = 1;
static char *little_endian = (char *) & _endian;

int
#ifdef __STDC__
bdf_little_endian(void)
#else
bdf_little_endian()
#endif
{
	return *little_endian;
}
