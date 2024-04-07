#include "xglobal.h"
#pragma hdrstop

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;

extern char* IErrMsg;

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

#define MAX_BITS		15
#define MAX_BL_BITS		7
#define LENGTH_CODES		29
#define LITERALS		256
#define END_BLOCK		256
#define L_CODES 		(LITERALS+1+LENGTH_CODES)
#define D_CODES 		30
#define BL_CODES		19

#define STORED_BLOCK		0
#define STATIC_TREES		1
#define DYN_TREES		2

#define LIT_BUFSIZE		0x4000

#define DIST_BUFSIZE		LIT_BUFSIZE

#define REP_3_6 		16
#define REPZ_3_10		17
#define REPZ_11_138		18

typedef struct ct_data {
	union {
		ushort freq;
		ushort code;
	} fc;
	union {
		ushort dad;
		ushort len;
	} dl;
} ct_data;

typedef struct tree_desc
{
	ct_data* dyn_tree;
	ct_data* static_tree;
	int* extra_bits;
	int extra_base;
	int elems;
	int max_length;
	int max_code;
} tree_desc;

#define Freq fc.freq
#define Code fc.code
#define Dad  dl.dad
#define Len  dl.len

#define HEAP_SIZE (2 * L_CODES + 1)

//		---------- EXTERN SECTION ----------

extern int verbose;
extern int level;

//	       ---------- PROTOTYPE SECTION ----------

void ct_init(ushort* attr,int* method);
ulong flush_block(char* buf,ulong stored_len,int eof);
int ct_tally(int dist,int lc);

void copy_block(char* buf,unsigned len,int header);
void bi_windup();
unsigned bi_reverse(unsigned code,int len);
void send_bits(int value,int length);
void free_data();
void lm_free();

static void init_block();
static void pqdownheap(ct_data* tree, int k);
static void gen_bitlen(tree_desc* desc);
static void gen_codes(ct_data* tree, int max_code);
static void build_tree(tree_desc* desc);
static void scan_tree(ct_data* tree, int max_code);
static void send_tree(ct_data* tree, int max_code);
static int  build_bl_tree();
static void send_all_trees(int lcodes, int dcodes, int blcodes);
static void compress_block(ct_data* ltree, ct_data* dtree);
static void set_file_type();

//	       ---------- DEFINITION SECTION ----------

static int extra_lbits[LENGTH_CODES]
   = {0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0};

static int extra_dbits[D_CODES]
   = {0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};

static int extra_blbits[BL_CODES]
   = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,3,7};

static ct_data dyn_ltree[HEAP_SIZE];
static ct_data dyn_dtree[2*D_CODES+1];

static ct_data static_ltree[L_CODES+2];
static ct_data static_dtree[D_CODES];
static ct_data bl_tree[2*BL_CODES+1];

static tree_desc l_desc =
{dyn_ltree, static_ltree, extra_lbits, LITERALS + 1, L_CODES, MAX_BITS, 0};

static tree_desc d_desc =
{dyn_dtree, static_dtree, extra_dbits, 0,	   D_CODES, MAX_BITS, 0};

static tree_desc bl_desc =
{bl_tree, NULL,       extra_blbits, 0,	       BL_CODES, MAX_BL_BITS, 0};

static ushort bl_count[MAX_BITS + 1];

static uchar bl_order[BL_CODES]
   = {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};

static int heap[2 * L_CODES + 1];
static int heap_len;
static int heap_max;

static uchar depth[2 * L_CODES + 1];
static uchar length_code[MAX_MATCH - MIN_MATCH + 1];
static uchar dist_code[512];
static int base_length[LENGTH_CODES];

static int base_dist[D_CODES];

#ifndef DYN_ALLOC
	static uchar l_buf[LIT_BUFSIZE];
	static ushort d_buf[DIST_BUFSIZE];
#else
	static uchar* l_buf;
	static ushort* d_buf;
#endif

static uchar flag_buf[(LIT_BUFSIZE/8)];

static unsigned last_lit;
static unsigned last_dist;
static unsigned last_flags;
static uchar flags;
static uchar flag_bit;

static ulong opt_len;
static ulong static_len;

static ulong compressed_len;

static ulong input_len;

ushort *file_type;
int *file_method;

extern long block_start;
extern unsigned strstart;

#  define send_code(c, tree) \
	{ if(verbose>1) fprintf(stderr,"\ncd %3d ",(c)); \
	send_bits(tree[c].Code, tree[c].Len); }

#define d_code(dist) \
	((dist) < 256 ? dist_code[dist] : dist_code[256+((dist)>>7)])

#define MAX(a,b) (a >= b ? a : b)

void ct_init(ushort* attr,int* method)
{
	int n;
	int bits;
	int length;
	int code;
	int dist;

	file_type = attr;
	file_method = method;
	compressed_len = input_len = 0L;

#ifdef DYN_ALLOC
	d_buf = new ushort[DIST_BUFSIZE];
	l_buf = new uchar[LIT_BUFSIZE];
#endif
	length = 0;
	for(code = 0; code < LENGTH_CODES-1; code++){
		base_length[code] = length;
		for(n = 0; n < (1<<extra_lbits[code]); n++){
			length_code[length++] = (uchar)code;
		}
	}
	length_code[length-1] = (uchar)code;

	dist = 0;
	for(code = 0 ; code < 16; code++){
		base_dist[code] = dist;
		for(n = 0; n < (1<<extra_dbits[code]); n++){
			dist_code[dist++] = (uchar)code;
		}
	}
	dist >>= 7;
	for( ; code < D_CODES; code++){
		base_dist[code] = dist << 7;
		for(n = 0; n < (1<<(extra_dbits[code]-7)); n++){
			dist_code[256 + dist++] = (uchar)code;
		}
	}

	for(bits = 0; bits <= MAX_BITS; bits++) bl_count[bits] = 0;
	n = 0;
	while(n <= 143) static_ltree[n++].Len = 8, bl_count[8]++;
	while(n <= 255) static_ltree[n++].Len = 9, bl_count[9]++;
	while(n <= 279) static_ltree[n++].Len = 7, bl_count[7]++;
	while(n <= 287) static_ltree[n++].Len = 8, bl_count[8]++;
	gen_codes((ct_data*)static_ltree, L_CODES+1);

	for(n = 0; n < D_CODES; n++){
		static_dtree[n].Len = 5;
		static_dtree[n].Code = bi_reverse(n, 5);
	}

	init_block();
}

static void init_block()
{
	int n;

	for(n = 0; n < L_CODES; n++) dyn_ltree[n].Freq = 0;
	for(n = 0; n < D_CODES; n++) dyn_dtree[n].Freq = 0;
	for(n = 0; n < BL_CODES; n++) bl_tree[n].Freq = 0;

	dyn_ltree[END_BLOCK].Freq = 1;
	opt_len = static_len = 0L;
	last_lit = last_dist = last_flags = 0;
	flags = 0; flag_bit = 1;
}

#define SMALLEST	1

#define pqremove(tree, top) \
{\
	top = heap[SMALLEST]; \
	heap[SMALLEST] = heap[heap_len--]; \
	pqdownheap(tree,SMALLEST); \
}

#define smaller(tree, n, m) \
	(tree[n].Freq < tree[m].Freq || \
	(tree[n].Freq == tree[m].Freq && depth[n] <= depth[m]))

static void pqdownheap(ct_data* tree,int k)
{
	int v = heap[k];
	int j = k << 1;
	int htemp;

	while(j <= heap_len){
		if(j < heap_len && smaller(tree, heap[j+1], heap[j])) j++;

		htemp = heap[j];
		if(smaller(tree, v, htemp)) break;

		heap[k] = htemp;
		k = j;

		j <<= 1;
	}
	heap[k] = v;
}

static void gen_bitlen(tree_desc* desc)
{
	ct_data* tree	    = desc->dyn_tree;
	int* extra     = desc->extra_bits;
	int base	    = desc->extra_base;
	int max_code	    = desc->max_code;
	int max_length	    = desc->max_length;
	ct_data* stree = desc->static_tree;
	int h;
	int n, m;
	int bits;
	int xbits;
	ushort f;
	int overflow = 0;

	for(bits = 0; bits <= MAX_BITS; bits++) bl_count[bits] = 0;

	tree[heap[heap_max]].Len = 0;

	for(h = heap_max+1; h < HEAP_SIZE; h++){
		n = heap[h];
		bits = tree[tree[n].Dad].Len + 1;
		if(bits > max_length) bits = max_length, overflow++;
		tree[n].Len = bits;

		if(n > max_code) continue;

		bl_count[bits]++;
		xbits = 0;
		if(n >= base) xbits = extra[n-base];
		f = tree[n].Freq;
		opt_len += (ulong)f * (bits + xbits);
		if(stree) static_len += (ulong)f * (stree[n].Len + xbits);
	}
	if(overflow == 0) return;

	do {
		bits = max_length-1;
		while(bl_count[bits] == 0) bits--;
		bl_count[bits]--;
		bl_count[bits+1] += 2;
		bl_count[max_length]--;
		overflow -= 2;
	} while(overflow > 0);

	for(bits = max_length; bits != 0; bits--){
		n = bl_count[bits];
		while(n != 0){
			m = heap[--h];
			if(m > max_code) continue;
			if(tree[m].Len != (unsigned) bits){
				opt_len += ((long)bits-(long)tree[m].Len)*(long)tree[m].Freq;
				tree[m].Len = bits;
			}
			n--;
		}
	}
}

static void gen_codes (ct_data* tree,int max_code)
{
	ushort next_code[MAX_BITS+1];
	ushort code = 0;
	int bits;
	int n;

	for(bits = 1; bits <= MAX_BITS; bits++){
		next_code[bits] = code = (code + bl_count[bits-1]) << 1;
	}
	for(n = 0;  n <= max_code; n++){
		int len = tree[n].Len;
		if(len == 0) continue;
		tree[n].Code = bi_reverse(next_code[len]++, len);
	}
}

static void build_tree(tree_desc* desc)
{
	ct_data* tree	= desc->dyn_tree;
	ct_data* stree	= desc->static_tree;
	int elems	     = desc->elems;
	int n, m;
	int max_code = -1;
	int node = elems;

	heap_len = 0, heap_max = HEAP_SIZE;

	for(n = 0; n < elems; n++){
		if(tree[n].Freq != 0){
			heap[++heap_len] = max_code = n;
			depth[n] = 0;
		}
		else {
			tree[n].Len = 0;
		}
	}

	while(heap_len < 2){
		int new_ = heap[++heap_len] = (max_code < 2 ? ++max_code : 0);
		tree[new_].Freq = 1;
		depth[new_] = 0;
		opt_len--;
		if(stree)
			static_len -= stree[new_].Len;
	}
	desc->max_code = max_code;

	for(n = heap_len/2; n >= 1; n--) pqdownheap(tree, n);

	do {
		pqremove(tree, n);
		m = heap[SMALLEST];

		heap[--heap_max] = n;
		heap[--heap_max] = m;

		tree[node].Freq = tree[n].Freq + tree[m].Freq;
		depth[node] = (uchar) (MAX(depth[n], depth[m]) + 1);
		tree[n].Dad = tree[m].Dad = node;
		heap[SMALLEST] = node++;
		pqdownheap(tree, SMALLEST);

	} while(heap_len >= 2);

	heap[--heap_max] = heap[SMALLEST];
	gen_bitlen((tree_desc*)desc);
	gen_codes ((ct_data*)tree, max_code);
}

static void scan_tree (ct_data* tree,int max_code)
{
	int n;
	int prevlen = -1;
	int curlen;
	int nextlen = tree[0].Len;
	int count = 0;
	int max_count = 7;
	int min_count = 4;

	if(nextlen == 0) max_count = 138, min_count = 3;
	tree[max_code+1].Len = (ushort)-1;

	for(n = 0; n <= max_code; n++){
		curlen = nextlen; nextlen = tree[n+1].Len;
		if(++count < max_count && curlen == nextlen){
			continue;
		}
		else
			if(count < min_count){
				bl_tree[curlen].Freq += count;
			}
			else
				if(curlen != 0){
					if(curlen != prevlen) bl_tree[curlen].Freq++;
					bl_tree[REP_3_6].Freq++;
				}
				else
					if(count <= 10){
						bl_tree[REPZ_3_10].Freq++;
					}
					else {
						bl_tree[REPZ_11_138].Freq++;
					}
		count = 0; prevlen = curlen;
		if(nextlen == 0){
			max_count = 138, min_count = 3;
		} else
		if(curlen == nextlen){
			max_count = 6, min_count = 3;
		}
		else {
			max_count = 7, min_count = 4;
		}
	}
}

static void send_tree (ct_data* tree,int max_code)
{
	int n;
	int prevlen = -1;
	int curlen;
	int nextlen = tree[0].Len;
	int count = 0;
	int max_count = 7;
	int min_count = 4;

	if(nextlen == 0) max_count = 138, min_count = 3;

	for(n = 0; n <= max_code; n++){
		curlen = nextlen; nextlen = tree[n+1].Len;
		if(++count < max_count && curlen == nextlen){
			continue;
		}
		else
			if(count < min_count){
				do { send_code(curlen, bl_tree); } while(--count != 0);

			}
			else
				if(curlen != 0){
					if(curlen != prevlen){
						send_code(curlen, bl_tree); count--;
					}
					send_code(REP_3_6, bl_tree); send_bits(count-3, 2);

				}
				else
					if(count <= 10){
						send_code(REPZ_3_10, bl_tree); send_bits(count-3, 3);

					}
					else {
						send_code(REPZ_11_138, bl_tree); send_bits(count-11, 7);
					}
		count = 0; prevlen = curlen;
		if(nextlen == 0){
			max_count = 138, min_count = 3;
		}
		else
			if(curlen == nextlen){
				max_count = 6, min_count = 3;
			}
			else {
				max_count = 7, min_count = 4;
			}
	}
}

static int build_bl_tree()
{
	int max_blindex;

	scan_tree((ct_data*)dyn_ltree, l_desc.max_code);
	scan_tree((ct_data*)dyn_dtree, d_desc.max_code);

	build_tree((tree_desc*)(&bl_desc));
	for(max_blindex = BL_CODES-1; max_blindex >= 3; max_blindex--){
		if(bl_tree[bl_order[max_blindex]].Len != 0) break;
	}
	opt_len += 3*(max_blindex+1) + 5+5+4;

	return max_blindex;
}

static void send_all_trees(int lcodes,int dcodes,int blcodes)
{
	int rank;

	send_bits(lcodes-257, 5);
	send_bits(dcodes-1,   5);
	send_bits(blcodes-4,  4);
	for(rank = 0; rank < blcodes; rank++){
		send_bits(bl_tree[bl_order[rank]].Len, 3);
	}

	send_tree((ct_data*)dyn_ltree, lcodes-1);
	send_tree((ct_data*)dyn_dtree, dcodes-1);
}

ulong flush_block(char* buf,ulong stored_len,int eof)
{
	ulong opt_lenb, static_lenb;
	int max_blindex;

	flag_buf[last_flags] = flags;

	if(*file_type == (ushort)UNKNOWN) set_file_type();

	build_tree((tree_desc*)(&l_desc));

	build_tree((tree_desc*)(&d_desc));
	max_blindex = build_bl_tree();

	opt_lenb = (opt_len+3+7)>>3;
	static_lenb = (static_len+3+7)>>3;
	input_len += stored_len;
	if(static_lenb <= opt_lenb) opt_lenb = static_lenb;

	if(stored_len <= opt_lenb && eof && compressed_len == 0L){
		if(buf == NULL) ErrH.Abort(IErrMsg);
		copy_block(buf,(unsigned)stored_len,0);
		compressed_len = stored_len << 3;
		*file_method = STORE;
	}
	else
		if(stored_len+4 <= opt_lenb && buf != (char*)NULL){
			send_bits((STORED_BLOCK<<1)+eof, 3);
			compressed_len = (compressed_len + 3 + 7) & ~7L;
			compressed_len += (stored_len + 4) << 3;

			copy_block(buf, (unsigned)stored_len, 1);
			}
			else
				if(static_lenb == opt_lenb){
					send_bits((STATIC_TREES<<1)+eof, 3);
					compress_block((ct_data*)static_ltree, (ct_data*)static_dtree);
					compressed_len += 3 + static_len;
				}
				else {
					send_bits((DYN_TREES<<1)+eof, 3);
					send_all_trees(l_desc.max_code+1, d_desc.max_code+1, max_blindex+1);
					compress_block((ct_data*)dyn_ltree, (ct_data*)dyn_dtree);
					compressed_len += 3 + opt_len;
				}
	init_block();
	if(eof){
#ifdef DYN_ALLOC
		extern uchar *window;
#else
		extern uchar window[];
#endif
		memset(window, 0, (unsigned)(2*WSIZE-1));
		bi_windup();
		compressed_len += 7;
	}
	return compressed_len >> 3;
}


int ct_tally(int dist,int lc)
{
	l_buf[last_lit++] = (uchar)lc;
	if(dist == 0){
		dyn_ltree[lc].Freq++;
	}
	else {
		dist--;

		dyn_ltree[length_code[lc]+LITERALS+1].Freq++;
		dyn_dtree[d_code(dist)].Freq++;

		d_buf[last_dist++] = dist;
		flags |= flag_bit;
	}
	flag_bit <<= 1;

	if((last_lit & 7) == 0){
		flag_buf[last_flags++] = flags;
		flags = 0, flag_bit = 1;
	}
	if(level > 2 && (last_lit & 0xfff) == 0){
		ulong out_length = (ulong)last_lit*8L;
		ulong in_length = (ulong)strstart-block_start;
		int dcode;
		for(dcode = 0; dcode < D_CODES; dcode++){
			out_length += (ulong)dyn_dtree[dcode].Freq*(5L+extra_dbits[dcode]);
		}
		out_length >>= 3;
		if(last_dist < last_lit/2 && out_length < in_length/2) return 1;
	}
	return (last_lit == LIT_BUFSIZE-1 || last_dist == DIST_BUFSIZE);
}

static void compress_block(ct_data* ltree,ct_data* dtree)
{
	unsigned dist;
	int lc;
	unsigned lx = 0;
	unsigned dx = 0;
	unsigned fx = 0;
	uchar flag = 0;
	unsigned code;
	int extra;

	if(last_lit != 0) do {
		if((lx & 7) == 0) flag = flag_buf[fx++];
		lc = l_buf[lx++];
		if((flag & 1) == 0){
			send_code(lc, ltree);
		}
		else {
			code = length_code[lc];
			send_code(code+LITERALS+1, ltree);
			extra = extra_lbits[code];
			if(extra != 0){
				lc -= base_length[code];
				send_bits(lc, extra);
			}
			dist = d_buf[dx++];
			code = d_code(dist);

			send_code(code, dtree);
			extra = extra_dbits[code];
			if(extra != 0){
				dist -= base_dist[code];
				send_bits(dist, extra);
			}
		}
		flag >>= 1;
	} while(lx < last_lit);
	send_code(END_BLOCK, ltree);
}

static void set_file_type()
{
	int n = 0;
	unsigned ascii_freq = 0;
	unsigned bin_freq = 0;
	while(n < 7)	    bin_freq += dyn_ltree[n++].Freq;
	while(n < 128)	  ascii_freq += dyn_ltree[n++].Freq;
	while(n < LITERALS) bin_freq += dyn_ltree[n++].Freq;
	*file_type = bin_freq > (ascii_freq >> 2) ? BINARY : ASCII;
}

void free_data()
{
#ifdef DYN_ALLOC
	delete l_buf;
	delete d_buf;
	lm_free();
#endif
}
