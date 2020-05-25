/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/agt/agility.h"
#include "glk/quetzal.h"
#include "common/textconsole.h"

namespace Glk {
namespace AGT {

/* This includes wrappers for malloc, realloc, strdup, and free
     that exit gracefully if we run out of memory. */
/* There are also various utilities:
    concdup:  Creates a new string that is concatation of two others.
    strcasecmp: case insensitive comparison of strings
    strncasecmp: case insensitive compare of first n characters of strings
    fixsign16, fixsign32: routines to assemble signed ints out of
          individual bytes in an endian-free way. */
/* Also buffered file i/o routines and some misc. file utilites. */

#ifdef force16
#undef int
#endif

#ifdef force16
#define int short
#endif

long rangefix(long n) {
	if (n > 0) return n;
	return 0;
}

/*-------------------------------------------------------------------*/
/* Sign fixing routines, to build a signed 16- and 32-bit quantities */
/* out of their component bytes.                                     */
/*-------------------------------------------------------------------*/

#ifndef FAST_FIXSIGN
short fixsign16(uchar n1, uchar n2) {
	rbool sflag;
	short n;

	if (n2 > 0x80) {
		n2 &= 0x7F;
		sflag = 1;
	} else sflag = 0;

	n = n1 + (n2 << 8);
	if (sflag) n = n - 0x7fff - 1;
	return n;
}

long fixsign32(uchar n1, uchar n2, uchar n3, uchar n4) {
	rbool sflag;
	long n;

	if (n4 > 0x80) {
		n4 &= 0x7F;
		sflag = 1;
	} else sflag = 0;

	n = n1 + (((long)n2) << 8) + (((long)n3) << 16) + (((long)n4) << 24);
	if (sflag) n = n - 0x7fffffffL - 1L;
	return n;
}
#endif



/*----------------------------------------------------------------------*/
/* rprintf(), uses writestr for output           */
/*  This function is used mainly for diagnostic information */
/* There should be no newlines in the format string or in any of the */
/*  arguments as those could confuse writestr, except for the last */
/*  character in the string which can be a newline. */
/*----------------------------------------------------------------------*/

void rprintf(const char *fmt, ...) {
	int i;
	char s[100];
	va_list args;

	va_start(args, fmt);
	vsprintf(s, fmt, args);
	va_end(args);
	i = strlen(s) - 1;
	if (i >= 0 && s[i] == '\n') {
		s[i] = 0;
		writeln(s);
	} else writestr(s);
}


/*----------------------------------------------------------------------*/
/* Memory allocation wrappers: All memory allocation should run through */
/* these routines, which trap error conditions and do accounting to     */
/* help track down memory leaks.                                        */
/*----------------------------------------------------------------------*/

rbool rm_trap = 1;

long get_rm_size(void)
/* Return the amount of space being used by dynamically allocated things */
{
#ifdef MEM_INFO
	struct mstats memdata;

	memdata = mstats();
	return memdata.bytes_used;
#endif
	return 0;
}

long get_rm_freesize(void)
/* Return estimate of amount of space left */
{
#ifdef MEM_INFO
	struct mstats memdata;

	memdata = mstats();
	return memdata.bytes_free;
#endif
	return 0;
}


void *rmalloc(long size) {
	void *p;

	if (size > MAXSTRUC) {
		error("Memory allocation error: Over-sized structure requested.");
	}
	assert(size >= 0);
	if (size == 0) return NULL;
	p = malloc((size_t)size);
	if (p == NULL && rm_trap && size > 0) {
		error("Memory allocation error: Out of memory.");
	}
	if (rm_acct) ralloc_cnt++;
	return p;
}

void *rrealloc(void *old, long size) {
	void *p;

	if (size > MAXSTRUC) {
		error("Memory reallocation error: Oversized structure requested.");
	}
	assert(size >= 0);
	if (size == 0) {
		r_free(old);
		return NULL;
	}
	if (rm_acct && old == NULL) ralloc_cnt++;
	p = realloc(old, (size_t)size);
	if (p == NULL && rm_trap && size > 0) {
		error("Memory reallocation error: Out of memory.");
	}
	return p;
}

char *rstrdup(const char *s) {
	if (s == NULL) return NULL;

	char *t = scumm_strdup(s);
	if (t == NULL && rm_trap) {
		error("Memory duplication error: Out of memory.");
	}
	if (rm_acct) ralloc_cnt++;

	return t;
}

void r_free(void *p) {
	int tmp;

	if (p == NULL) return;

	tmp = get_rm_size();            /* Take worst case in all cases */
	if (tmp > rm_size) rm_size = tmp;
	tmp = get_rm_freesize();
	if (tmp < rm_freesize) rm_freesize = tmp;

	if (rm_acct) rfree_cnt++;
	free(p);
}



/*----------------------------------------------------------------------*/
/* String utilities: These are utilities to manipulate strings.         */
/*----------------------------------------------------------------------*/

/* rnstrncpy copies src to dest, copying at most (max-1) characters.
   Unlike ANSI strncpy, it doesn't fill extra space will nulls and
   it always puts a terminating null. */
char *rstrncpy(char *dest, const char *src, int max) {
	int i;
	for (i = 0; i < max - 1 && src[i]; i++)
		dest[i] = src[i];
	dest[i] = 0;
	return dest;
}

/* This does a case-insensitive match of the beginning of *pstr to match */
/* <match> must be all upper case */
/* *pstr is updated to point after the match, if it is succesful.
   Otherwise *pstr is left alone. */
rbool match_str(const char **pstr, const char *match) {
	int i;
	const char *s;

	s = *pstr;
	for (i = 0; match[i] != 0 && s[i] != 0; i++)
		if (toupper(s[i]) != match[i]) return 0;
	if (match[i] != 0) return 0;
	*pstr += i;
	return 1;
}




/* Utility to concacate two strings with a space inserted */

char *concdup(const char *s1, const char *s2) {
	int len1, len2;
	char *s;

	len1 = len2 = 0;
	if (s1 != NULL) len1 = strlen(s1);
	if (s2 != NULL) len2 = strlen(s2);

	s = (char *)rmalloc(sizeof(char) * (len1 + len2 + 2));
	if (s1 != NULL)
		memcpy(s, s1, len1);
	memcpy(s + len1, " ", 1);
	if (s2 != NULL)
		memcpy(s + len1 + 1, s2, len2);
	s[len1 + len2 + 1] = 0;
	return s;
}


/* Misc. C utility functions that may be supported locally.
  If they are, use the local functions since they'll probably be faster
  and more efficiant. */

#ifdef NEED_STR_CMP
int strcasecmp(const char *s1, const char *s2)
/* Compare strings s1 and s2, case insensitive; */
/* If equal, return 0. Otherwise return nonzero. */
{
	int i;

	for (i = 0; tolower(s1[i]) == tolower(s2[i]) && s1[i] != 0; i++);
	if (tolower(s1[i]) == tolower(s2[i])) return 0;
	if (s1[i] == 0) return -1;
	if (s2[i] == 0) return 1;
	if (tolower(s1[i]) < tolower(s2[i])) return -1;
	return 1;
}
#endif /* NEED_STR_CMP */

#ifdef NEED_STRN_CMP
int strncasecmp(const char *s1, const char *s2, size_t n)
/* Compare first n letters of strings s1 and s2, case insensitive; */
/* If equal, return 0. Otherwise return nonzero. */
{
	size_t i;

	for (i = 0; i < n && tolower(s1[i]) == tolower(s2[i]) && s1[i] != 0; i++);
	if (i == n || tolower(s1[i]) == tolower(s2[i])) return 0;
	if (s1[i] == 0) return -1;
	if (s2[i] == 0) return 1;
	if (tolower(s1[i]) < tolower(s2[i])) return -1;
	return 1;
}
#endif /* NEED_STRN_CMP */

/*----------------------------------------------------------------------*/
/* Character utilities: Do character translation                        */
/*----------------------------------------------------------------------*/

void build_trans_ascii(void) {
	int i;

	for (i = 0; i < 256; i++)
		trans_ascii[i] = (!fix_ascii_flag || i < 0x80) ? i : trans_ibm[i & 0x7f];
	trans_ascii[0xFF] = 0xFF; /* Preserve format character */
}


/*----------------------------------------------------------------------*/
/* File utilities: Utilities to manipulate files.                       */
/*----------------------------------------------------------------------*/

void print_error(const char *fname, filetype ext, const char *err, rbool ferr) {
	char *estring; /* Hold error string */
	estring = (char *)rmalloc(strlen(err) + strlen(fname) + 2);
	sprintf(estring, err, fname);
	if (ferr) fatal(estring);
	else writeln(estring);
	rfree(estring);
}

/* Routine to open files with extensions and handle basic error conditions */

genfile fopen(const char *name, const char *how) {
	if (!strcmp(how, "r") || !strcmp(how, "rb")) {
		Common::File *f = new Common::File();
		if (!f->open(name)) {
			delete f;
			f = nullptr;
		}

		return f;
	} else if (!strcmp(how, "w") || !strcmp(how, "wb")) {
		Common::DumpFile *f = new Common::DumpFile();
		if (!f->open(name)) {
			delete f;
			f = nullptr;
		}

		return f;
	} else {
		error("Unknown file open how");
	}
}

int fseek(genfile stream, long int offset, int whence) {
	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(stream);
	assert(rs);
	return rs->seek(offset, whence);
}

size_t fread(void *ptr, size_t size, size_t nmemb, genfile stream) {
	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(stream);
	assert(rs);
	size_t bytesRead = rs->read(ptr, size * nmemb);
	return bytesRead / size;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, genfile stream) {
	Common::WriteStream *ws = dynamic_cast<Common::WriteStream *>(stream);
	assert(ws);
	size_t bytesWritten = ws->write(ptr, size * nmemb);
	return bytesWritten / size;
}

size_t ftell(genfile f) {
	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(f);
	assert(rs);
	return rs->pos();
}

genfile openfile(fc_type fc, filetype ext, const char *err, rbool ferr)
/* Opens the file fname+ext, printing out err if something goes wrong.
  (unless err==NULL, in which case nothing will be printed) */
/* err can have one %s paramater in it, which will have the file name
   plugged in to it. */
/* If ferr is true, then on failure the routine will abort with a fatal
   error. */
{
	genfile tfile;  /* Actually, this may not be a text file anymore */
	const char *errstr;

	tfile = readopen(fc, ext, &errstr);
	if (errstr != NULL && err != NULL)
		print_error("", ext, err, ferr);

	return tfile;
}


genfile openbin(fc_type fc, filetype ext, const char *err, rbool ferr)
/* Opens the file fname+ext, printing out err if something goes wrong.
  (unless err==NULL, in which case nothing will be printed) */
/* err can have one %s paramater in it, which will have the file name
   plugged in to it. */
/* If ferr is true, then on failure the routine will abort with a fatal
   error. */
{
	genfile f;  /* Actually, this may not be a text file anymore */
	const char *errstr;
	char *fname;

	f = readopen(fc, ext, &errstr);
	if (errstr != NULL && err != NULL) {
		fname = formal_name(fc, ext);
		print_error(fname, ext, err, ferr);
		rfree(fname);
	}

	return f;
}



/* This routine reads in a line from a 'text' file; it's designed to work
   regardless of the EOL conventions of the platform, at least up to a point.
   It should work with files that have \n, \r, or \r\n termined lines.  */

#define READLN_GRAIN 64 /* Granularity of readln() rrealloc requests
                this needs to be at least the size of a tab
                character */
#define DOS_EOF 26    /* Ctrl-Z is the DOS end-of-file marker */

char *readln(genfile f, char *buff, int n)
/* Reads first n characters of line, eliminates any newline,
and truncates the rest of the line. 'n' does *not* include terminating
null. */
/* If we pass it BUFF=NULL,  then it will reallocate buff as needed and
pass it back as its return value.  n is ignored in this case */
/* If it reaches EOF, it will return NULL */
/* This routine recognizes lines terminated by \n, \r, or \r\n */
{
	int c;
	int i, j, csize;
	int buffsize; /* Current size of buff, if we are allocating it dynamically */

	if (buff == NULL) {
		buff = (char *)rrealloc(buff, READLN_GRAIN * sizeof(char));
		buffsize = READLN_GRAIN;
		n = buffsize - 1;
	} else buffsize = -1; /* So we know that we are using a fixed-size buffer */

	i = 0;
	for (;;) {
		c = textgetc(f);

		if (c == '\n' || c == '\r' || c == EOF || c == DOS_EOF) break;

		csize = (c == '\t') ? 5 : 1; /* Tabs are translated into five spaces */

		if (i + csize >= n && buffsize >= 0) {
			buffsize += READLN_GRAIN;
			n = buffsize - 1;
			buff = (char *)rrealloc(buff, buffsize * sizeof(char));
		}

		if (c == 0) c = FORMAT_CODE;
		else if (c != '\t') {
			if (i < n) buff[i++] = c;
		} else for (j = 0; j < 5 && i < n; j++) buff[i++] = ' ';

		/* We can't exit the loop if i>n since we still need to discard
		   the rest of the line */
	}

	buff[i] = 0;

	if (c == '\r') { /* Check for \r\n DOS-style newline  */
		char newc;
		newc = textgetc(f);
		if (newc != '\n') textungetc(f, newc);
		/* Replace the character we just read. */
	} else if (c == DOS_EOF) /* Ctrl-Z is the DOS EOF marker */
		textungetc(f, c); /* So it will be the first character we see next time */

	if (i == 0 && (c == EOF || c == DOS_EOF)) { /* We've hit the end of the file */
		if (buffsize >= 0) rfree(buff);
		return NULL;
	}

	if (buffsize >= 0) { /* Shrink buffer to appropriate size */
		buffsize = i + 1;
		buff = (char *)rrealloc(buff, buffsize);
	}

	return buff;
}


/*-------------------------------------------------------------------------*/
/* Buffered file Input: Routines to do buffered file I/O for files organized */
/*  into records.  These routines are highly non-reentrant: they use a     */
/*  global buffer and a global file id, so only they can only access one   */
/*  file at a time.                                                        */
/* buffopen() should not be called on a new file until buffclose has been  */
/*  called on the old one.                                                 */
/*-------------------------------------------------------------------------*/

genfile bfile;

static uchar *buffer = NULL;
static long buffsize; /* How big the buffer is */
static long record_size;  /* Size of a record in the file */
static long buff_frame;  /* The file index corrosponding to buffer[0] */
static long buff_fcnt;  /* Number of records that can be held in the buffer */
static long real_buff_fcnt;  /* Number of records actually held in buffer */
static long buff_rsize;  /* Minimum amount that must be read. */

static long block_size;  /* Size of the current block
                (for non-AGX files, this is just the filesize) */
static long block_offset; /* Offset of current block in file (this should
               be zero for non-AGX files) */


static void buff_setrecsize(long recsize) {
	const char *errstr;

	record_size = recsize;
	real_buff_fcnt = buff_fcnt = buffsize / record_size;
	buff_frame = 0;

	/* Note that real_buff_cnt==buff_fcnt in this case because
	   the buffer will have already been resized to be <=
	   the block size-- so we don't need to worry about the
	   buffer being larger than the data we're reading in. */

	binseek(bfile, block_offset);
	if (!binread(bfile, buffer, record_size, real_buff_fcnt, &errstr))
		fatal(errstr);
}



long buffopen(fc_type fc, filetype ext, long minbuff, const char *rectype, long recnum)
/* Returns record size; print out error and halt on failure  */
/* (if agx_file, it returns the filesize instead) */
/* rectype="noun","room", etc.  recnum=number of records expected */
/* If rectype==NULL, buffopen() will return 0 on failure instead of
halting */
/* For AGX files, recsize should be set to minbuff... but
buffreopen will be called before any major file activity
(in particular, recnum should be 1) */
{
	long filesize;
	long recsize;
	char ebuff[200];
	const char *errstr;

	assert(buffer == NULL); /* If not, it means these routines have been
               called by someone else who isn't done yet */

	bfile = readopen(fc, ext, &errstr);
	if (errstr != NULL) {
		if (rectype == NULL) {
			return 0;
		} else
			fatal(errstr);
	}

	filesize = binsize(bfile);

	block_size = filesize;
	block_offset = 0;
	if (agx_file) block_size = minbuff; /* Just for the beginning */

	if (block_size % recnum != 0) {
		sprintf(ebuff, "Fractional record count in %s file.", rectype);
		agtwarn(ebuff, 0);
	}
	buff_rsize = recsize = block_size / recnum;
	if (buff_rsize > minbuff) buff_rsize = minbuff;

	/* No point in having a buffer bigger than the block size */
	buffsize = BUFF_SIZE;
	if (block_size < buffsize) buffsize = block_size;

	/* ... but it needs to be big enough: */
	if (buffsize < minbuff) buffsize = minbuff;
	if (buffsize < recsize) buffsize = recsize;

	buffer = (uchar *)rmalloc(buffsize); /* Might want to make this adaptive eventually */

	buff_setrecsize(recsize);
	if (!agx_file && DIAG) {
		char *s;
		s = formal_name(fc, ext);
		rprintf("Reading %s file %s (size:%ld)\n", rectype, s, filesize);
		rfree(s);
		rprintf("  Record size=  Formal:%ld    File:%ld", minbuff, recsize);
	}
	if (agx_file) return (long) filesize;
	else return (long) recsize;
}


/* Compute the game signature: a checksum of relevant parts of the file */

static void compute_sig(uchar *buff) {
	long bp;
	for (bp = 0; bp < buff_rsize; bp++)
		game_sig = (game_sig + buff[bp]) & 0xFFFF;
}

uchar *buffread(long index) {
	uchar *bptr;
	const char *errstr;

	assert(buff_rsize <= record_size);
	if (index >= buff_frame && index < buff_frame + real_buff_fcnt)
		bptr = buffer + (index - buff_frame) * record_size;
	else {
		binseek(bfile, block_offset + index * record_size);
		real_buff_fcnt = block_size / record_size - index;  /* How many records
                             could we read in? */
		if (real_buff_fcnt > buff_fcnt)
			real_buff_fcnt = buff_fcnt; /* Don't overflow buffer */
		if (!binread(bfile, buffer, record_size, real_buff_fcnt, &errstr))
			fatal(errstr);
		buff_frame = index;
		bptr = buffer;
	}
	if (!agx_file) compute_sig(bptr);
	return bptr;
}


void buffclose(void) {
	readclose(bfile);
	rfree(buffer);
}


/* This changes the record size and offset settings of the buffered
   file so we can read files that consist of multiple sections with
   different structures */
static void buffreopen(long f_ofs, long file_recsize, long recnum,
                       long bl_size, const char *rectype) {
	char ebuff[200];
	long recsize;

	/* Compute basic statistics */
	block_offset = f_ofs; /* Offset of this block */
	block_size = bl_size; /* Size of the entire block (all records) */
	if (block_size % recnum != 0) {
		/* Check that the number of records divides the block size evenly */
		sprintf(ebuff, "Fractional record count in %s block.", rectype);
		agtwarn(ebuff, 0);
	}
	buff_rsize = recsize = block_size / recnum;
	if (buff_rsize > file_recsize) buff_rsize = file_recsize;
	/* recsize is the size of each record in the file.
	   buff_rsize is the internal size of each record (the part
	   we actually look at, which may be smaller than recsize) */

	/* No point in having a buffer bigger than the block size */
	buffsize = BUFF_SIZE;
	if (block_size < buffsize) buffsize = block_size;

	/* The buffer needs to be at least as big as one block, so
	   we have space to both read it in and so we can look at the
	   block without having to worry about how big it really is */
	if (buffsize < file_recsize) buffsize = file_recsize;
	if (buffsize < recsize) buffsize = recsize;

	rfree(buffer);
	buffer = (uchar *)rmalloc(buffsize); /* Resize the buffer */

	buff_setrecsize(recsize); /* Set up remaining stats */
}


/*-------------------------------------------------------------------------*/
/* Buffered file output: Routines to buffer output  for files organized    */
/*  into records.  These routines are highly non-reentrant: they use a     */
/*  global buffer and a global file id, so only they can only access one   */
/*  file at a time.                                                        */
/* This routine uses the same buffer and data structures as the reading    */
/*  routines above, so  both sets of routines should not be used           */
/*  concurrently                                                           */
/*-------------------------------------------------------------------------*/

/* #define DEBUG_SEEK*/  /* Debug seek beyond EOF problem */

static long bw_first, bw_last;  /* First and last record in buffer written to.
                 This is relative to the beginning of the
                 buffer bw_last points just beyond the last
                 one written to */
#ifdef DEBUG_SEEK
static long bw_fileleng;  /* Current file length */
#endif /* DEBUG_SEEK */
file_id_type bw_fileid;

/* Unlike is reading counterpart, this doesn't actually allocate
   a buffer; that's done by bw_setblock() which should be called before
   any I/O */
void bw_open(fc_type fc, filetype ext) {
	const char *errstr;

	assert(buffer == NULL);

	bfile = writeopen(fc, ext, &bw_fileid, &errstr);
	if (errstr != NULL) fatal(errstr);
	bw_last = 0;
	buffsize = 0;
	buffer = NULL;
#ifdef DEBUG_SEEK
	bw_fileleng = 0;
#endif
}

static void bw_seek(long offset) {
#ifdef DEBUG_SEEK
	assert(offset <= bw_fileleng);
#endif
	binseek(bfile, offset);
}


static void bw_flush(void) {
	if (bw_first == bw_last) return; /* Nothing to do */
	bw_first += buff_frame;
	bw_last += buff_frame;
	bw_seek(block_offset + bw_first * record_size);
	binwrite(bfile, buffer, record_size, bw_last - bw_first, 1);
#ifdef DEBUG_SEEK
	if (block_offset + bw_last * record_size > bw_fileleng)
		bw_fileleng = block_offset + bw_last * record_size;
#endif
	bw_first = bw_last = 0;
}



static void bw_setblock(long fofs, long recnum, long rsize)
/* Set parameters for current block */
{
	/* First, flush old block if neccessary */
	if (buffer != NULL) {
		bw_flush();
		rfree(buffer);
	}
	block_size = rsize * recnum;
	block_offset = fofs;
	record_size = rsize;
	buff_frame = 0;
	bw_first = bw_last = 0;
	buffsize = BUFF_SIZE;
	if (buffsize > block_size) buffsize = block_size;
	if (buffsize < rsize) buffsize = rsize;
	buff_fcnt = buffsize / rsize;
	buffsize = buff_fcnt * rsize;
	buffer = (uchar *)rmalloc(buffsize);
}

/* This routine returns a buffer of the current recsize and with
 the specified index into the file */
/* The buffer will be written to disk after the next call to
   bw_getbuff() or bw_closebuff() */
static uchar *bw_getbuff(long index) {
	index -= buff_frame;
	if (index < bw_first || index > bw_last || index >= buff_fcnt) {
		bw_flush();
		bw_first = bw_last = 0;
		buff_frame = buff_frame + index;
		index = 0;
	}
	if (index == bw_last) bw_last++;
	return buffer + record_size * index;
}


/* This flushes all buffers to disk and closes all files */
void bw_close(void) {
	bw_flush();
	rfree(buffer);
	writeclose(bfile, bw_fileid);
}

void bw_abort(void) {
	binremove(bfile, bw_fileid);
}


/*-------------------------------------------------------------------------*/
/* Block reading and writing code and support for internal buffers         */
/*-------------------------------------------------------------------------*/


/* If the internal buffer is not NULL, it is used instead of a file */
/* (This is used by RESTART, etc. to save state to memory rather than
   to a file) */
static uchar *int_buff = NULL;
static long ibuff_ofs, ibuff_rsize;

void set_internal_buffer(void *buff) {
	int_buff = (uchar *)buff;
}

static void set_ibuff(long offset, long rsize) {
	ibuff_ofs = offset;
	record_size = ibuff_rsize = rsize;
}

static uchar *get_ibuff(long index) {
	return int_buff + ibuff_ofs + index * ibuff_rsize;
}

/* This does a block write to the currently buffered file.
   At the moment this itself does no buffering at all; it's intended
   for high speed reading of blocks of chars for which we've already
   allocated the space. */
static void buff_blockread(void *buff, long size, long offset) {
	const char *errstr;

	if (int_buff != NULL)
		memcpy((char *)buff, int_buff + offset, size);
	else {
		binseek(bfile, offset);
		if (!binread(bfile, buff, size, 1, &errstr)) fatal(errstr);
	}
}


/* This writes buff to disk. */
static void bw_blockwrite(void *buff, long size, long offset) {
	if (int_buff != NULL)
		memcpy(int_buff + offset, (char *)buff, size);
	else {
		bw_flush();
		bw_seek(offset);
		binwrite(bfile, buff, size, 1, 1);
#ifdef DEBUG_SEEK
		if (offset + size > bw_fileleng) bw_fileleng = offset + size;
#endif
	}
}


/*-------------------------------------------------------------------------*/
/* Platform-independent record-based file I/O: Routines to read and write  */
/*   files according to the file_info data structures.                     */
/* These routines use the buffered I/O routines above                      */
/*-------------------------------------------------------------------------*/

/* Length of file datatypes */
const size_t ft_leng[FT_COUNT] = {0, 2, 2, /* END, int16, and uint16 */
                                  4, 4,  /* int32 and uint32 */
                                  1, 2, 0, /* byte, version, rbool */
                                  8, 4,  /* descptr, ss_ptr */
                                  2, 26,  /* slist, path[13] */
                                  4, 4, /* cmdptr, dictptr */
                                  81,  /* tline */
                                  1, 1
                                 };  /* char, cfg */


long compute_recsize(file_info *recinfo) {
	long cnt, bcnt;

	cnt = 0;
	for (; recinfo->ftype != FT_END; recinfo++)
		if (recinfo->ftype == FT_BOOL) {
			for (bcnt = 0; recinfo->ftype == FT_BOOL; recinfo++, bcnt++);
			recinfo--;
			cnt += (bcnt + 7) / 8; /* +7 is to round up */
		} else
			cnt += ft_leng[recinfo->ftype];
	return cnt;
}

static const int agx_version[] = {0, 0000, 1800, 2000, 3200, 3500, 8200, 8300,
                                  5000, 5050, 5070, 10000, 10050, 15000, 15500, 16000, 20000
                                 };

static int agx_decode_version(int vercode) {
	if (vercode & 1) /* Large/Soggy */
		if (vercode == 3201) ver = 4;
		else ver = 2;
	else if (vercode < 10000) ver = 1;
	else ver = 3;
	switch (vercode & (~1)) {
	case 0000:
		return AGT10;
	case 1800:
		return AGT118;
	case 1900:
		return AGT12;
	case 2000:
		return AGT12;
	case 3200:
		return AGTCOS;
	case 3500:
		return AGT135;
	case 5000:
		return AGT15;
	case 5050:
		return AGT15F;
	case 5070:
		return AGT16;
	case 8200:
		return AGT182;
	case 8300:
		return AGT183;
	case 8350:
		return AGT183;
	case 10000:
		return AGTME10;
	case 10050:
		return AGTME10A;
	case 15000:
		return AGTME15;
	case 15500:
		return AGTME155;
	case 16000:
		return AGTME16;
	case 20000:
		return AGX00;
	default:
		agtwarn("Unrecognize AGT version", 0);
		return 0;
	}
}

/* The following reads a section of a file into variables, doing
   the neccessary conversions. It is the foundation of all the generic
   file reading code */

#define p(t)  ((t*)(rec_desc->ptr))
#define fixu16(n1,n2)  ( ((long)(n1))|( ((long)(n2))<<8 ))

/* This is as large as the largest data structure we could run into */
static const uchar zero_block[81] = {0, 0, 0, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0,
                                     0
                                    };

static void read_filerec(file_info *rec_desc, const uchar *filedata) {
	uchar mask;
	rbool past_eob; /* Are we past the end of block? */
	const uchar *filebase;

	mask = 1;
	past_eob = 0;
	filebase = filedata;
	for (; rec_desc->ftype != FT_END; rec_desc++) {
		if (mask != 1 && rec_desc->ftype != FT_BOOL) { /* Just finished rboolean */
			mask = 1;
			filedata += 1;
		}
		if (filebase == NULL || (filedata - filebase) >= record_size) {
			/* We're past the end of the block; read in zeros for the rest
			of entries. */
			past_eob = 1;
			filedata = zero_block;
			filebase = NULL;
		}
		switch (rec_desc->ftype) {
		case FT_INT16:
			if (rec_desc->dtype == DT_LONG)
				*p(long) = fixsign16(filedata[0], filedata[1]);
			else
				*p(integer) = fixsign16(filedata[0], filedata[1]);
			break;
		case FT_UINT16:
			*p(int32) = fixu16(filedata[0], filedata[1]);
			break;
		case FT_CMDPTR: /* cmd ptr */
		case FT_INT32:
			*p(int32) = fixsign32(filedata[0], filedata[1],
			                     filedata[2], filedata[3]);
			break;
		case FT_UINT32:
			if (filedata[3] & 0x80)
				agtwarn("File value out of range", 0);
			*p(uint32) = fixsign32(filedata[0], filedata[1],
			                     filedata[2], filedata[3] & 0x7F);
			break;
		case FT_BYTE:
			*p(uchar) = filedata[0];
			break;
		case FT_CHAR:
			*p(uchar) = trans_ascii[filedata[0]^'r'];
			break;
		case FT_VERSION:
			*p(int) = agx_decode_version(fixu16(filedata[0], filedata[1]));
			break;
		case FT_CFG:
			if (filedata[0] != 2 && !past_eob)
				*p(rbool) = filedata[0];
			break;
		case FT_BOOL:
			*p(rbool) = ((filedata[0] & mask) != 0);
			if (mask == 0x80) {
				filedata++;
				mask = 1;
			} else
				mask <<= 1;
			break;
		case FT_DESCPTR:
			if (skip_descr) break;
			p(descr_ptr)->start = fixsign32(filedata[0], filedata[1],
			                                filedata[2], filedata[3]);
			p(descr_ptr)->size = fixsign32(filedata[4], filedata[5],
			                               filedata[6], filedata[7]);
			break;
		case FT_STR:  /* ss_string ptr */
			*p(char *) = static_str + fixsign32(filedata[0], filedata[1],
			                                    filedata[2], filedata[3]);
			break;
		case FT_SLIST:
			*p(slist) = fixsign16(filedata[0], filedata[1]);
			break;
		case FT_PATHARRAY: { /* integer array[13] */
			int i;
			for (i = 0; i < 13; i++)
				p(integer)[i] = fixsign16(filedata[2 * i], filedata[2 * i + 1]);
			break;
		}
		case FT_TLINE: { /* string of length at most 80 characters +null */
			uchar *s;
			int i;
			s = (uchar *)*p(tline);
			for (i = 0; i < 80; i++)
				s[i] = trans_ascii[filedata[i]^'r'];
			s[80] = 0;
			break;
		}
		case FT_DICTPTR: /* ptr into dictstr */
			*p(char *) = dictstr + fixsign32(filedata[0], filedata[1],
			                                 filedata[2], filedata[3]);
			break;
		default:
			fatal("Unreconized field type");
		}
		filedata += ft_leng[rec_desc->ftype];
	}
}


#define v(t) (*(t*)(rec_desc->ptr))
/* Here is the corresponding routien for _writing_ to files */
/* This copies the contents of a record into a buffer */

static void write_filerec(file_info *rec_desc, uchar *filedata) {
	uchar mask;

	mask = 1;
	for (; rec_desc->ftype != FT_END; rec_desc++) {
		if (mask != 1 && rec_desc->ftype != FT_BOOL) { /* Just finished rboolean */
			mask = 1;
			filedata += 1;
		}
		switch (rec_desc->ftype) {
		case FT_INT16:
			if (rec_desc->dtype == DT_LONG) {
				filedata[0] = v(long) & 0xFF;
				filedata[1] = (v(long) >> 8) & 0xFF;
			} else {
				filedata[0] = v(integer) & 0xFF;
				filedata[1] = (v(integer) >> 8) & 0xFF;
			}
			break;
		case FT_UINT16:
			filedata[0] = v(long) & 0xFF;
			filedata[1] = (v(long) >> 8) & 0xFF;
			break;
		case FT_CMDPTR: /* cmd ptr */
		case FT_INT32:
		case FT_UINT32:
			filedata[0] = v(long) & 0xFF;
			filedata[1] = (v(long) >> 8) & 0xFF;
			filedata[2] = (v(long) >> 16) & 0xFF;
			filedata[3] = (v(long) >> 24) & 0xFF;
			break;
		case FT_BYTE:
			filedata[0] = v(uchar);
			break;
		case FT_CFG:
			filedata[0] = v(uchar);
			break;
		case FT_CHAR:
			filedata[0] = v(uchar)^'r';
			break;
		case FT_VERSION: {
			int tver;
			tver = agx_version[v(int)];
			if (ver == 2 || ver == 4) tver += 1;
			filedata[0] = tver & 0xFF;
			filedata[1] = (tver >> 8) & 0xFF;
			break;
		}
		case FT_BOOL:
			if (mask == 1) filedata[0] = 0;
			filedata[0] |= v(rbool) ? mask : 0;
			if (mask == 0x80) {
				filedata++;
				mask = 1;
			} else
				mask <<= 1;
			break;
		case FT_DESCPTR: {
			long i, n1, n2;
			n1 = p(descr_ptr)->start;
			n2 = p(descr_ptr)->size;
			for (i = 0; i < 4; i++) {
				filedata[i] = n1 & 0xFF;
				filedata[i + 4] = n2 & 0xFF;
				n1 >>= 8;
				n2 >>= 8;
			}
		}
		break;
		case FT_STR: { /* ss_string ptr */
			long delta;
			delta = v(char *) - static_str;
			filedata[0] = delta & 0xFF;
			filedata[1] = (delta >> 8) & 0xFF;
			filedata[2] = (delta >> 16) & 0xFF;
			filedata[3] = (delta >> 24) & 0xFF;
			break;
		}
		case FT_SLIST:
			filedata[0] = v(slist) & 0xFF;
			filedata[1] = (v(slist) >> 8) & 0xFF;
			break;
		case FT_PATHARRAY: { /* integer array[13] */
			int i;
			for (i = 0; i < 13; i++) {
				filedata[2 * i] = *(p(integer) + i) & 0xFF;
				filedata[2 * i + 1] = (*(p(integer) + i) >> 8) & 0xFF;
			}
			break;
		}
		case FT_TLINE: { /* string of length at most 80 characters +null */
			uchar *s;
			int i;
			s = (uchar *)v(tline);
			for (i = 0; i < 80; i++)
				filedata[i] = s[i]^'r';
			filedata[80] = 0;
			break;
		}
		case FT_DICTPTR: { /* ptr into dictstr */
			long delta;
			delta = v(char *) - dictstr;
			filedata[0] = delta & 0xFF;
			filedata[1] = (delta >> 8) & 0xFF;
			filedata[2] = (delta >> 16) & 0xFF;
			filedata[3] = (delta >> 24) & 0xFF;
			break;
		}
		default:
			fatal("Unreconized field type");
		}
		filedata += ft_leng[rec_desc->ftype];
	}
}

#undef v
#undef p




/* This reads in a structure array */
/* base=the beginning of the array. If NULL, this is malloc'd and returned
   eltsize = the size of each structure
   numelts = the number of elements in the array
   field_info = the arrangement of fields within the strucutre
   rectype = string to print out for error messages
   file_offset = the offset of the beginning of the array into the file
   */
void *read_recarray(void *base, long eltsize, long numelts,
                    file_info *field_info, const char *rectype,
                    long file_offset, long file_blocksize) {
	long i;
	file_info *curr;
	uchar *file_data;

	if (numelts == 0) return NULL;

	if (int_buff)
		set_ibuff(file_offset, compute_recsize(field_info));
	else
		buffreopen(file_offset, compute_recsize(field_info), numelts,
		           file_blocksize, rectype);

	if (base == NULL)
		base = rmalloc(eltsize * numelts);

	for (curr = field_info; curr->ftype != FT_END; curr++)
		if (curr->dtype != DT_DESCPTR && curr->dtype != DT_CMDPTR)
			curr->ptr = ((char *)base + curr->offset);

	for (i = 0; i < numelts; i++) {
		if (!int_buff)
			file_data = buffread(i);
		else
			file_data = get_ibuff(i);
		read_filerec(field_info, file_data);
		for (curr = field_info; curr->ftype != FT_END; curr++)
			if (curr->dtype == DT_DESCPTR)
				curr->ptr = (char *)(curr->ptr) + sizeof(descr_ptr);
			else if (curr->dtype == DT_CMDPTR)
				curr->ptr = (char *)(curr->ptr) + sizeof(long);
			else
				curr->ptr = (char *)(curr->ptr) + eltsize;
	}

	return base;
}


/* A NULL value means to write junk; we're just producing
   a placeholder for systems that can't seek beyond the end-of-file */

long write_recarray(void *base, long eltsize, long numelts,
                    file_info *field_info, long file_offset) {
	long i;
	file_info *curr;
	uchar *file_data;

	if (numelts == 0) return 0;

	if (int_buff)
		set_ibuff(file_offset, compute_recsize(field_info));
	else
		bw_setblock(file_offset, numelts, compute_recsize(field_info));

	if (base != NULL)
		for (curr = field_info; curr->ftype != FT_END; curr++)
			if (curr->dtype != DT_DESCPTR && curr->dtype != DT_CMDPTR)
				curr->ptr = ((char *)base + curr->offset);

	for (i = 0; i < numelts; i++) {
		if (int_buff)
			file_data = get_ibuff(i);
		else
			file_data = bw_getbuff(i);
		if (base != NULL) {
			write_filerec(field_info, file_data);
			for (curr = field_info; curr->ftype != FT_END; curr++)
				if (curr->dtype == DT_DESCPTR)
					curr->ptr = (char *)(curr->ptr) + sizeof(descr_ptr);
				else if (curr->dtype == DT_CMDPTR)
					curr->ptr = (char *)(curr->ptr) + sizeof(long);
				else
					curr->ptr = (char *)(curr->ptr) + eltsize;
		}
	}
	return compute_recsize(field_info) * numelts;
}


void read_globalrec(file_info *global_info, const char *rectype,
                    long file_offset, long file_blocksize) {
	uchar *file_data;

	if (int_buff) {
		set_ibuff(file_offset, compute_recsize(global_info));
		file_data = get_ibuff(0);
	} else {
		buffreopen(file_offset, compute_recsize(global_info), 1, file_blocksize,
		           rectype);
		file_data = buffread(0);
	}
	read_filerec(global_info, file_data);
}


long write_globalrec(file_info *global_info, long file_offset) {
	uchar *file_data;

	if (int_buff) {
		set_ibuff(file_offset, compute_recsize(global_info));
		file_data = get_ibuff(0);
	} else {
		bw_setblock(file_offset, 1, compute_recsize(global_info));
		file_data = bw_getbuff(0);
	}
	write_filerec(global_info, file_data);
	return compute_recsize(global_info);
}



static file_info fi_temp[] = {
	{0, DT_DEFAULT, NULL, 0},
	endrec
};

/* This routine reads in an array of simple data */

void *read_recblock(void *base, int ftype, long numrec,
                    long offset, long bl_size) {
	int dsize;

	switch (ftype) {
	case FT_CHAR:
	case FT_BYTE:
		if (base == NULL) base = rmalloc(numrec * sizeof(char));
		buff_blockread(base, numrec, offset);
		if (ftype == FT_CHAR) {
			long i;
			for (i = 0; i < numrec; i++)
				((uchar *)base)[i] = trans_ascii[((uchar *)base)[i]^'r' ];
		}
		return base;
	case FT_SLIST:
		dsize = sizeof(slist);
		break;
	case FT_INT16:
		dsize = sizeof(integer);
		break;
	case FT_UINT16:
	case FT_INT32:
		dsize = sizeof(long);
		break;
	case FT_STR:
	case FT_DICTPTR:
		dsize = sizeof(char *);
		break;
	default:
		fatal("Invalid argument to read_recblock.");
		dsize = 0; /* Silence compiler warnings; this will never actually
          be reached. */
	}

	fi_temp[0].ftype = ftype;
	return read_recarray(base, dsize, numrec, fi_temp, "", offset, bl_size);
}


long write_recblock(void *base, int ftype, long numrec, long offset) {
	int dsize;

	if (numrec == 0) return 0;
	switch (ftype) {
	case FT_CHAR: {
		int i;
		for (i = 0; i < numrec; i++)
			((uchar *)base)[i] = ((uchar *)base)[i]^'r';
	}
	/* Fall through.... */
	case FT_BYTE:
		bw_blockwrite(base, numrec, offset);
		return numrec;
	case FT_SLIST:
		dsize = sizeof(slist);
		break;
	case FT_INT16:
		dsize = sizeof(integer);
		break;
	case FT_INT32:
		dsize = sizeof(long);
		break;
	case FT_STR:
	case FT_DICTPTR:
		dsize = sizeof(char *);
		break;
	default:
		fatal("Invalid argument to write_recblock.");
		dsize = 0; /* Silence compiler warnings; this will never actually
          be reached. */
	}

	fi_temp[0].ftype = ftype;
	return write_recarray(base, dsize, numrec, fi_temp, offset);
}

char *textgets(genfile f, char *buf, size_t n) {
	Common::ReadStream *rs = dynamic_cast<Common::ReadStream *>(f);
	assert(rs);

	size_t count = 0;
	char c;

	while (!rs->eos() && (count < (n - 1)) && (c = rs->readByte()) != '\n') {
		buf[count] = c;
		++count;
	}

	buf[count] = '\0';
	return count ? buf : nullptr;
}

char textgetc(genfile f) {
	Common::ReadStream *rs = dynamic_cast<Common::ReadStream *>(f);
	assert(rs);

	return rs->eos() ? EOF : rs->readByte();
}

void textungetc(genfile f, char c) {
	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(f);
	assert(rs);

	rs->seek(-1, SEEK_SET);
}

bool texteof(genfile f) {
	Common::ReadStream *rs = dynamic_cast<Common::ReadStream *>(f);
	assert(rs);

	return rs->eos();
}

void textputs(genfile f, const char *s) {
	Common::WriteStream *ws = dynamic_cast<Common::WriteStream *>(f);
	assert(ws);

	ws->write(s, strlen(s));
}

/* ------------------------------------------------------------------- */
/* "Profiling" functions             */
/* Routines for timing code execution */
/* These will only work on POSIX systems */

#ifdef PROFILE_SUPPORT

static struct tms start;
clock_t start_realtime;
static struct tms delta;
clock_t delta_realtime;

void resetwatch(void) {
	delta.tms_utime = delta.tms_stime = delta.tms_cutime = delta.tms_cstime = 0;
	delta_realtime = 0;
	start_realtime = times(&start);
}

void startwatch(void) {
	start_realtime = times(&start);
}

static char watchbuff[81];
char *timestring(void) {
	sprintf(watchbuff, "User:%ld.%02ld   Sys:%ld.%02ld   Total:%ld.%02ld"
	        "   Real:%ld.%02ld",
	        delta.tms_utime / 100, delta.tms_utime % 100,
	        delta.tms_stime / 100, delta.tms_stime % 100,
	        (delta.tms_utime + delta.tms_stime) / 100,
	        (delta.tms_utime + delta.tms_stime) % 100,
	        delta_realtime / 100, delta_realtime % 100
	       );
	return watchbuff;
}

char *stopwatch(void) {
	struct tms curr;

	delta_realtime += times(&curr) - start_realtime;
	delta.tms_utime += (curr.tms_utime - start.tms_utime);
	delta.tms_stime += (curr.tms_stime - start.tms_stime);
	delta.tms_cutime += (curr.tms_cutime - start.tms_cutime);
	delta.tms_cstime += (curr.tms_cstime - start.tms_cstime);
	return timestring();
}

/* 5+7+9+8+4*3+4*?? = 41+?? */

#endif /* PROFILE_SUPPORT */

} // End of namespace AGT
} // End of namespace Glk
