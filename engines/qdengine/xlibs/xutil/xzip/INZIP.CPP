#include "xglobal.h"
#pragma hdrstop

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;

struct huft;

#ifdef _LARGE_MODEL_
#define WSIZE		0x4000U
#define OUTBUFSIZ	0x800U
#else
#define WSIZE		0x8000U
#define OUTBUFSIZ	0x1000U
#endif

#define MIN_MATCH	3
#define MAX_MATCH	258

#define DYN_ALLOC
#define MIN_LOOKAHEAD	(MAX_MATCH + MIN_MATCH + 1)
#define MAX_DIST	(WSIZE-MIN_LOOKAHEAD)

#define UNKNOWN 	(-1)
#define BINARY		0
#define ASCII		1

#define STORE		0
#define DEFLATE 	8


#define Buf_size (8 * 2 * sizeof(char))

#define PUTSHORT(w)							\
{	if(out_offset < out_size-1){					\
		out_buf[out_offset++] = (char)((w) & 0xFF);		\
		out_buf[out_offset++] = (char)((ushort)(w) >> 8);	\
	} else {							\
		flush_outbuf((w),2);					\
	}								\
}

#define PUTBYTE(b)							\
{	if(out_offset < out_size){					\
		out_buf[out_offset++] = (char)(b);			\
	} else {							\
		flush_outbuf((b),1);					\
	}								\
}

//		---------- EXTERN SECTION ----------

extern ulong window_size;

//	       ---------- PROTOTYPE SECTION ----------

ulong deflate();

void lm_init(int pack_level,ushort* flags);
void bi_init();
void bi_windup();
unsigned bi_reverse(unsigned code,int len);

void ct_init(ushort* attr,int* method);

void send_bits(int value,int length);
void flush_outbuf(unsigned w,unsigned bytes);
int mem_read(char* b,unsigned bsize);
void copy_block(char* buf,unsigned len,int header);
void free_data();

//	       ---------- DEFINITION SECTION ----------

unsigned short bi_buf;

char* IErrMsg = "ZIP Internal Error";

int level = 6;
int verbose = 0;
int bi_valid;

char* in_buf;
char* out_buf;

unsigned in_offset;
unsigned out_offset;
unsigned in_size;
unsigned out_size;

int (*read_buf)(char* buf,unsigned size);

ulong ZIP_compress(char* trg,ulong trgsize,char* src,ulong srcsize)
{
	ushort att = (ushort)UNKNOWN;
	ushort flags = 0;
	int method = DEFLATE;

	read_buf = mem_read;
	in_buf = src;
	in_size = (unsigned)srcsize;
	in_offset = 0;

	out_buf = trg;
	out_size = (unsigned)trgsize;
	out_offset = 2 + 4;
	window_size = 0L;

	if(out_size <= 6) return 0;

	bi_init();
	ct_init(&att,&method);
	lm_init((level != 0 ? level : 1),&flags);
	deflate();
	free_data();
	window_size = 0L;

	trg[0] = (char)(method & 0xFF);
	trg[1] = (char)((method >> 8) & 0xFF);

	*(ulong*)(trg + 2) = srcsize;
	return (ulong)out_offset;
}

void bi_init()
{
	bi_buf = 0;
	bi_valid = 0;
}

void send_bits(int value,int length)
{
	if(bi_valid > (int)Buf_size - length){
		bi_buf |= (value << bi_valid);
		PUTSHORT(bi_buf);
		bi_buf = (ushort)value >> (Buf_size - bi_valid);
		bi_valid += length - Buf_size;
	}
	else {
		bi_buf |= value << bi_valid;
		bi_valid += length;
	}
}

unsigned bi_reverse(unsigned code,int len)
{
	register unsigned res = 0;
	do {
		res |= code & 1;
		code >>= 1, res <<= 1;
	} while (--len > 0);
	return res >> 1;
}

void flush_outbuf(unsigned w,unsigned bytes)
{
	ErrH.Abort(IErrMsg);
}

void bi_windup()
{
	if(bi_valid > 8){
		PUTSHORT(bi_buf);
	}
	else
		if(bi_valid > 0){
			PUTBYTE(bi_buf);
		}
	bi_buf = 0;
	bi_valid = 0;
}

void copy_block(char* buf,unsigned len,int header)
{
	bi_windup();
	if(header){
		PUTSHORT((ushort)len);
		PUTSHORT((ushort)~len);
	}
	if(out_offset + len > out_size){
		ErrH.Abort(IErrMsg);
	}
	else {
		memcpy(out_buf + out_offset, buf, len);
		out_offset += len;
	}
}

int mem_read(char* b,unsigned bsize)
{
	if(in_offset < in_size){
		ulong block_size = in_size - in_offset;
		if (block_size > (ulong)bsize) block_size = (ulong)bsize;
		memcpy(b, in_buf + in_offset, (unsigned)block_size);
		in_offset += (unsigned)block_size;
		return (int)block_size;
	}
	else {
		return 0;
	}
}



#ifdef _LARGE_MODEL_
#define HASH_BITS	14
#else
#define HASH_BITS	15
#endif

#define HASH_SIZE	(unsigned)(1<<HASH_BITS)
#define HASH_MASK	(HASH_SIZE-1)
#define WMASK		(WSIZE-1)

#define NIL		0

#define FAST		4
#define SLOW		2

#ifndef TOO_FAR
#	define TOO_FAR	4096
#endif

typedef ushort		Pos;
typedef unsigned	IPos;

#define H_SHIFT 	((HASH_BITS+MIN_MATCH-1)/MIN_MATCH)


//		---------- EXTERN SECTION ----------

extern int (*read_buf)(char* buf,unsigned size);
extern int verbose;
extern int level;

//	       ---------- PROTOTYPE SECTION ----------

void fill_window();
ulong deflate_fast();
int longest_match(IPos cur_match);
void lm_init(int pack_level,ushort* flags);
void lm_free();
ulong deflate();
ulong flush_block(char* buf,ulong stored_len,int eof);
int ct_tally(int dist,int lc);

//	       ---------- DEFINITION SECTION ----------

#ifndef DYN_ALLOC
	uchar window[2L*WSIZE];
	Pos prev[WSIZE];
	Pos head[HASH_SIZE];
#else
	uchar* window = NULL;
	Pos* prev = NULL;
	Pos* head;
#endif

ulong window_size;
long block_start;
static int sliding;
static unsigned ins_h;

unsigned int prev_length;

unsigned strstart;
unsigned match_start;
static int eofile;
static unsigned lookahead;

unsigned max_chain_length;
static unsigned int max_lazy_match;

#define max_insert_length max_lazy_match

unsigned good_match;

typedef struct config {
	ushort good_length;
	ushort max_lazy;
	ushort nice_length;
	ushort max_chain;
} config;

int nice_match;

static config configuration_table[10] = {
/* 0 */ {0,    0,  0,	 0},
/* 1 */ {4,    4,  8,	 4},
/* 2 */ {4,    5, 16,	 8},
/* 3 */ {4,    6, 32,	32},

/* 4 */ {4,    4, 16,	16},
/* 5 */ {8,   16, 32,	32},
/* 6 */ {8,   16, 128, 128},
/* 7 */ {8,   32, 128, 256},
/* 8 */ {32, 128, 258, 1024},
/* 9 */ {32, 258, 258, 4096}};

#define EQUAL 0

#define UPDATE_HASH(h,c) (h = (((h)<<H_SHIFT) ^ (c)) & HASH_MASK)

#define INSERT_STRING(s, match_head)			\
	(UPDATE_HASH(ins_h, window[(s) + MIN_MATCH-1]), \
	prev[(s) & WMASK] = match_head = head[ins_h],	\
	head[ins_h] = (s))

void lm_init(int pack_level,ushort* flags)
{
	register unsigned j;

	if(pack_level < 1 || pack_level > 9) ErrH.Abort(IErrMsg);

	sliding = 0;
	if(window_size == 0L){
		sliding = 1;
		window_size = (ulong)2L*WSIZE;
	}

#ifdef DYN_ALLOC
	if(window == NULL)
		window = new uchar[WSIZE * 2];
	if(prev == NULL){
		prev = new Pos[WSIZE];
		head = new Pos[HASH_SIZE];
	}
#endif

	head[HASH_SIZE-1] = NIL;
	memset((char*)head,NIL,(unsigned)(HASH_SIZE - 1) * sizeof(*head));

	max_lazy_match = configuration_table[pack_level].max_lazy;
	good_match = configuration_table[pack_level].good_length;
	nice_match = configuration_table[pack_level].nice_length;
	max_chain_length = configuration_table[pack_level].max_chain;
	if(pack_level == 1){
		*flags |= FAST;
	} else
		if(pack_level == 9){
			*flags |= SLOW;
		}
	strstart = 0;
	block_start = 0L;

	j = WSIZE;

	if(sizeof(int) > 2)
		j <<= 1;

	lookahead = (*read_buf)((char*)window, j);

	if(lookahead == 0 || lookahead == (unsigned)EOF){
		eofile = 1, lookahead = 0;
		return;
	}
	eofile = 0;
	while(lookahead < MIN_LOOKAHEAD && !eofile) fill_window();

	ins_h = 0;
	for(j=0; j<MIN_MATCH-1; j++) UPDATE_HASH(ins_h, window[j]);
}

void lm_free()
{
#ifdef DYN_ALLOC
	if(window != NULL){
//		  free(window);
		delete window;
		window = NULL;
	}
	if(prev != NULL){
/*
		free((unsigned char*)prev);
		free((unsigned char*)head);
*/
		delete prev;
		delete head;
		prev = head = NULL;
	}
#endif
}

int longest_match(IPos cur_match)
{
	unsigned chain_length = max_chain_length;
	register uchar *scan = window + strstart;
	register uchar *match;
	register int len;
	int best_len = prev_length;
	IPos limit = strstart > (IPos)MAX_DIST ? strstart - (IPos)MAX_DIST : NIL;

	register uchar *strend = window + strstart + MAX_MATCH;
	register uchar scan_end1  = scan[best_len-1];
	register uchar scan_end   = scan[best_len];
	if(prev_length >= good_match){
		chain_length >>= 2;
	}
	do {
		match = window + cur_match;
		if(match[best_len]   != scan_end  ||
			match[best_len-1] != scan_end1 ||
			*match != *scan ||
			*++match != scan[1]) continue;

		scan += 2, match++;
		do {
		} while(*++scan == *++match && *++scan == *++match &&
			*++scan == *++match && *++scan == *++match &&
			*++scan == *++match && *++scan == *++match &&
			*++scan == *++match && *++scan == *++match &&
			scan < strend);

		len = MAX_MATCH - (int)(strend - scan);
		scan = strend - MAX_MATCH;
		if(len > best_len){
			match_start = cur_match;
			best_len = len;
			if(len >= nice_match) break;
			scan_end1  = scan[best_len-1];
			scan_end   = scan[best_len];
		}
	} while((cur_match = prev[cur_match & WMASK]) > limit && --chain_length != 0);
	return best_len;
}

#define check_match(start, match, length)

void fill_window()
{
	register unsigned n, m;
	unsigned more = (unsigned)(window_size - (ulong)lookahead - (ulong)strstart);
	if(more == (unsigned)EOF){
		more--;
	} else
		if(strstart >= WSIZE+MAX_DIST && sliding){
			memcpy((char*)window, (char*)window+WSIZE, (unsigned)WSIZE);
			match_start -= WSIZE;
			strstart    -= WSIZE; /* we now have strstart >= MAX_DIST: */

			block_start -= (long) WSIZE;

			for(n = 0; n < HASH_SIZE; n++){
				m = head[n];
				head[n] = (Pos)(m >= WSIZE ? m-WSIZE : NIL);
			}
			for(n = 0; n < WSIZE; n++){
				m = prev[n];
				prev[n] = (Pos)(m >= WSIZE ? m-WSIZE : NIL);
			}
			more += WSIZE;
			if(verbose) putc('.', stderr);
		}
	if(!eofile){
		n = (*read_buf)((char*)window+strstart+lookahead, more);
		if(n == 0 || n == (unsigned)EOF){
			eofile = 1;
		}
		else {
			lookahead += n;
		}
	}
}

#define FLUSH_BLOCK(eof) \
   flush_block(block_start >= 0L ? (char*)&window[(unsigned)block_start] : \
		(char*)NULL,(long)strstart - block_start,(eof))

ulong deflate_fast()
{
	IPos hash_head;
	int flush;
	unsigned match_length = 0;

	prev_length = MIN_MATCH-1;
	while(lookahead != 0){
		INSERT_STRING(strstart, hash_head);
		if(hash_head != NIL && strstart - hash_head <= MAX_DIST){
			match_length = longest_match (hash_head);
			if(match_length > lookahead) match_length = lookahead;
		}
		if(match_length >= MIN_MATCH){
			check_match(strstart, match_start, match_length);
			flush = ct_tally(strstart-match_start, match_length - MIN_MATCH);
			lookahead -= match_length;
			if(match_length <= max_insert_length){
				match_length--;
				do {
					strstart++;
					INSERT_STRING(strstart, hash_head);
				} while(--match_length != 0);
				strstart++;
			}
			else {
				strstart += match_length;
				match_length = 0;
				ins_h = window[strstart];
				UPDATE_HASH(ins_h, window[strstart+1]);
			}
		}
		else {
			flush = ct_tally (0, window[strstart]);
			lookahead--;
			strstart++;
		}
		if(flush) FLUSH_BLOCK(0), block_start = strstart;
		while(lookahead < MIN_LOOKAHEAD && !eofile) fill_window();
    }
	return FLUSH_BLOCK(1);
}

ulong deflate()
{
	IPos hash_head;
	IPos prev_match;
	int flush;
	int match_available = 0;
	register unsigned match_length = MIN_MATCH - 1;

	if(level <= 3) return deflate_fast();

	while(lookahead != 0){
		INSERT_STRING(strstart, hash_head);

		prev_length = match_length, prev_match = match_start;
		match_length = MIN_MATCH-1;

		if(hash_head != NIL && prev_length < max_lazy_match && strstart - hash_head <= MAX_DIST){
			match_length = longest_match (hash_head);
			if(match_length > lookahead) match_length = lookahead;

			if(match_length == MIN_MATCH && strstart-match_start > TOO_FAR){
				match_length--;
			}
		}
		if(prev_length >= MIN_MATCH && match_length <= prev_length){
			check_match(strstart-1, prev_match, prev_length);
			flush = ct_tally(strstart-1-prev_match, prev_length - MIN_MATCH);
			lookahead -= prev_length-1;
			prev_length -= 2;
			do {
				strstart++;
				INSERT_STRING(strstart, hash_head);
			} while(--prev_length != 0);
			match_available = 0;
			match_length = MIN_MATCH-1;
			strstart++;
			if(flush) FLUSH_BLOCK(0), block_start = strstart;
		}
		else
			if(match_available){
				if(ct_tally (0, window[strstart-1])){
					FLUSH_BLOCK(0), block_start = strstart;
				}
				strstart++;
				lookahead--;
			}
			else {
			match_available = 1;
			strstart++;
			lookahead--;
		}
		while(lookahead < MIN_LOOKAHEAD && !eofile) fill_window();
	}
	if(match_available) ct_tally(0, window[strstart-1]);
	return FLUSH_BLOCK(1);
}

