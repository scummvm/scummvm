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

//		---------- EXTERN SECTION ----------
//	       ---------- PROTOTYPE SECTION ----------

int FlushMemory();
int FlushOutput();
void inflate();
int ReadMemoryByte(ushort* x);

//	       ---------- DEFINITION SECTION ----------

uchar* outbuf;
uchar* outptr;

uchar* slide;

int outcnt = 0;
long outpos = 0;
ulong bitbuf = 0;
int bits_left = 0;
char zipeof = 0;

ushort mask_bits[] = {
    0x0000,
    0x0001, 0x0003, 0x0007, 0x000F, 0x001F, 0x003F, 0x007F, 0x00FF,
    0x01FF, 0x03FF, 0x07FF, 0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF
};

static uchar* mem_i_buffer;
static uchar* mem_o_buffer;
static ulong mem_i_size,mem_i_offset;
static ulong mem_o_size,mem_o_offset;

unsigned long ZIP_GetExpandedSize(char* p)
{
	return *(unsigned long*)(p + 2);
}

void ZIP_expand(char* trg,ulong trgsize,char* src,ulong srcsize)
{
	ushort method;

	outbuf = new uchar[OUTBUFSIZ + 1];
	slide = new uchar[WSIZE];

	method = *(short*)(src);

	mem_i_buffer = (uchar*)src + 2 + 4;
	mem_i_size   = srcsize - 2 - 4;
	mem_i_offset = 0;

	mem_o_buffer = (uchar*)trg;
	mem_o_size   = trgsize;
	mem_o_offset = 0;

	bits_left = 0;
	bitbuf = 0L;
	outpos = 0L;
	outcnt = 0;
	outptr = outbuf;
	zipeof = 0;
	memset(outbuf,0xAA,OUTBUFSIZ);

	switch (method) {
		case STORE:
			memcpy(trg,src + 2 + 4,(unsigned)(srcsize - 2 - 4));
		break;
		case DEFLATE:
			inflate();
			FlushOutput();
			break;
		default:
			break;
	}

	delete outbuf;
	delete slide;
}

int FlushOutput()
{
	int rc = FlushMemory();
	outpos += outcnt;
	outcnt = 0;
	outptr = outbuf;
	return rc;
}

int ReadMemoryByte(ushort* x)
{
	if (mem_i_offset < mem_i_size) {
		*x = (ushort) mem_i_buffer[mem_i_offset++];
		return 8;
	} else
		return 0;
}

int FlushMemory()
{
	if(outcnt == 0)
		return 0;

	if(mem_o_offset + outcnt <= mem_o_size){
		memcpy((char*)(mem_o_buffer + mem_o_offset),(char*)outbuf,outcnt);
		mem_o_offset += outcnt;
		return 0;
	} else
		return 50;
}


#define NEXTBYTE	(ReadByte(&bytebuf), bytebuf)
#define NEEDBITS(n)	{while(k<(n)){b|=((ulong)NEXTBYTE)<<k;k+=8;}}
#define DUMPBITS(n)	{b>>=(n);k-=(n);}

#define BMAX		16
#define N_MAX		288

//		---------- EXTERN SECTION ----------

extern uchar* outptr;
extern int outcnt;

//	       ---------- PROTOTYPE SECTION ----------

int FlushOutput();
int ReadMemoryByte(ushort* x);

int huft_build(unsigned* b,unsigned n,unsigned s,ushort* d,ushort* e,struct huft** t,int* m);
int huft_free(struct huft* t);
void flush(unsigned w);
int inflate_codes(struct huft* tl,struct huft* td,int bl,int bd);
int inflate_stored();
int inflate_fixed();
int inflate_dynamic();
int inflate_block(int* e);
int inflate_entry();
void inflate();
int ReadByte(ushort* x);

//	       ---------- DEFINITION SECTION ----------

struct huft {
	uchar e;
	uchar b;
	union {
		ushort n;
		struct huft *t;
	} v;
};

static unsigned border[] = {
	16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
static ushort cplens[] = {
	3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
	35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0};
static ushort cplext[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
	3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 99, 99};
static ushort cpdist[] = {
	1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
	257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
	8193, 12289, 16385, 24577};
static ushort cpdext[] = {
	0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
	7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
	12, 12, 13, 13};

unsigned wp;
ulong bb;
unsigned bk;
ushort bytebuf;

int lbits = 9;
int dbits = 6;

unsigned hufts;

int huft_build(unsigned* b,unsigned n,unsigned s,ushort* d,ushort* e,struct huft** t,int* m)
{
	unsigned a;
	unsigned c[BMAX+1];
	unsigned f;
	int g;
	int h;
	unsigned i;
	unsigned j;
	int k;
	int l;
	unsigned *p;
	struct huft *q;
	struct huft r;
	struct huft *u[BMAX];
	unsigned v[N_MAX];
	int w;
	unsigned x[BMAX+1];
	unsigned *xp;
	int y;
	unsigned z;

	memset(c, 0,sizeof(c));
	p = b;	i = n;
	do {
		c[*p++]++;
	} while(--i);
	if(c[0] == n)
		return 2;

	l = *m;
	for(j = 1; j <= BMAX; j++)
		if(c[j])
			break;
		k = j;
		if((unsigned)l < j)
			l = j;
		for(i = BMAX; i; i--)
			if(c[i])
				break;
		g = i;
		if((unsigned)l > i)
			l = i;
		*m = l;

		for(y = 1 << j; j < i; j++,y <<= 1)
			if((y -= c[j]) < 0)
				return 2;
		if((y -= c[i]) < 0)
			return 2;
		c[i] += y;

	x[1] = j = 0;
	p = c + 1;  xp = x + 2;
	while(--i){
		*xp++ = (j += *p++);
	}

	p = b;	i = 0;
	do {
		if((j = *p++) != 0)
			v[x[j]++] = i;
	} while(++i < n);

	x[0] = i = 0;
	p = v;
	h = -1;
	w = -l;
	u[0] = (struct huft*)NULL;
	q = (struct huft*)NULL;
	z = 0;

	for(; k <= g; k++){
		a = c[k];
		while(a--){
			while(k > w + l){
				h++;
				w += l;
				z = (z = g - w) > (unsigned)l ? l : z;
				if((f = 1 << (j = k - w)) > a + 1)
				{
					f -= a + 1;
					xp = c + k;
					while(++j < z)
					{
						if((f <<= 1) <= *++xp)
							break;
						f -= *xp;
					}
				}
				z = 1 << j;
				q = new huft[z + 1];
				hufts += z + 1;
				*t = q + 1;
				*(t = &(q->v.t)) = (struct huft*)NULL;
				u[h] = ++q;

				if(h){
					x[h] = i;
					r.b = (uchar)l;
					r.e = (uchar)(16 + j);
					r.v.t = q;
					j = i >> (w - l);
					u[h-1][j] = r;
				}
			}

			r.b = (uchar)(k - w);
			if(p >= v + n)
				r.e = 99;
			else
				if(*p < s){
					r.e = (uchar)(*p < 256 ? 16 : 15);
					r.v.n = *p++;
				}
			else {
				r.e = (uchar)e[*p - s];  /* non-simple--look up in lists */
				r.v.n = d[*p++ - s];
			}

			f = 1 << (k - w);
			for(j = i >> w; j < z; j += f)
				q[j] = r;

			for(j = 1 << (k - 1); i & j; j >>= 1)
				i ^= j;
			i ^= j;

			while((i & ((1 << w) - 1)) != x[h]){
				h--;
				w -= l;
			}
		}
	}
	return y != 0 && n != 1;
}

int huft_free(struct huft* t)
{
	struct huft *p,*q;
	p = t;
	while(p != (struct huft*)NULL){
		q = (--p)->v.t;
//		  free(p);
		delete p;
		p = q;
	}
	return 0;
}

void flush(unsigned w)
{
	unsigned n;
	uchar *p;

	p = slide;
	while(w){
		n = (n = OUTBUFSIZ - outcnt) < w ? n : w;
		memcpy(outptr,p,n);
		outptr += n;
		if((outcnt += n) == OUTBUFSIZ)
			FlushOutput();
		p += n;
		w -= n;
	}
}

int inflate_codes(struct huft* tl,struct huft* td,int bl,int bd)
{
	unsigned e;
	unsigned n,d;
	unsigned w;
	struct huft *t;
	unsigned ml,md;
	ulong b;
	unsigned k;

	b = bb;
	k = bk;
	w = wp;

	ml = mask_bits[bl];
	md = mask_bits[bd];
	while(1){
		NEEDBITS((unsigned)bl)
		if((e = (t = tl + ((unsigned)b & ml))->e) > 16)
			do {
				if(e == 99)
					return 1;
				DUMPBITS(t->b)
				e -= 16;
				NEEDBITS(e)
			} while((e = (t = t->v.t + ((unsigned)b & mask_bits[e]))->e) > 16);
			DUMPBITS(t->b)
			if(e == 16){
				slide[w++] = (uchar)t->v.n;
				if(w == WSIZE){
					flush(w);
					w = 0;
				}
			}
			else {
				if(e == 15)
					break;
				NEEDBITS(e)
				n = t->v.n + ((unsigned)b & mask_bits[e]);
				DUMPBITS(e);
				NEEDBITS((unsigned)bd)
				if((e = (t = td + ((unsigned)b & md))->e) > 16)
					do {
						if(e == 99)
							return 1;
						DUMPBITS(t->b)
						e -= 16;
						NEEDBITS(e)
					} while((e = (t = t->v.t + ((unsigned)b & mask_bits[e]))->e) > 16);
				DUMPBITS(t->b)
				NEEDBITS(e)
				d = w - t->v.n - ((unsigned)b & mask_bits[e]);
				DUMPBITS(e)
				do {
					n -= (e = (e = WSIZE - ((d &= WSIZE-1) > w ? d : w)) > n ? n : e);
					if(w - d >= e){
						memcpy(slide + w,slide + d,e);
						w += e;
						d += e;
					}
					else
						do {
							slide[w++] = slide[d++];
						} while(--e);
					if(w == WSIZE){
						flush(w);
						w = 0;
					}
				} while(n);
			}
		}
	wp = w;
	bb = b;
	bk = k;
	return 0;
}

int inflate_stored()
{
	unsigned n;
	unsigned w;
	ulong b;
	unsigned k;

	b = bb;
	k = bk;
	w = wp;
	n = k & 7;
	DUMPBITS(n);

	NEEDBITS(16)
	n = ((unsigned)b & 0xffff);
	DUMPBITS(16)
	NEEDBITS(16)
	if(n != (unsigned)((~b) & 0xffff))
		return 1;
	DUMPBITS(16)

	while(n--){
		NEEDBITS(8)
		slide[w++] = (uchar)b;
		if(w == WSIZE){
			flush(w);
			w = 0;
		}
		DUMPBITS(8)
	}

	wp = w;
	bb = b;
	bk = k;
	return 0;
}

int inflate_fixed()
{
	int i;
	struct huft *tl;
	struct huft *td;
	int bl;
	int bd;
	unsigned l[288];

	for(i = 0; i < 144; i++)
		l[i] = 8;
	for(; i < 256; i++)
		l[i] = 9;
	for(; i < 280; i++)
		l[i] = 7;
	for(; i < 288; i++)
		l[i] = 8;
	bl = 7;
	if((i = huft_build(l,288,257,cplens,cplext,&tl,&bl)) != 0)
		return i;

	for(i = 0; i < 30; i++)
		l[i] = 5;
	bd = 5;
	if((i = huft_build(l,30,0,cpdist,cpdext,&td,&bd)) > 1){
		huft_free(tl);
		return i;
	}

	if(inflate_codes(tl,td,bl,bd))
		return 1;

	huft_free(tl);
	huft_free(td);
	return 0;
}

int inflate_dynamic()
{
	int i;
	unsigned j;
	unsigned l;
	unsigned m;
	unsigned n;
	struct huft *tl;
	struct huft *td;
	int bl;
	int bd;
	unsigned nb;
	unsigned nl;
	unsigned nd;
	unsigned ll[286+30];
	ulong b;
	unsigned k;

	b = bb;
	k = bk;

	NEEDBITS(5)
	nl = 257 + ((unsigned)b & 0x1f);
	DUMPBITS(5)
	NEEDBITS(5)
	nd = 1 + ((unsigned)b & 0x1f);
	DUMPBITS(5)
	NEEDBITS(4)
	nb = 4 + ((unsigned)b & 0xf);
	DUMPBITS(4)
	if(nl > 286 || nd > 30)
		return 1;
	for(j = 0; j < nb; j++){
		NEEDBITS(3)
		ll[border[j]] = (unsigned)b & 7;
		DUMPBITS(3)
	}
	for(; j < 19; j++)
		ll[border[j]] = 0;
	bl = 7;
	if((i = huft_build(ll,19,19,NULL,NULL,&tl,&bl)) != 0){
		if(i == 1)
			huft_free(tl);
		return i;
	}
	n = nl + nd;
	m = mask_bits[bl];
	i = l = 0;
	while((unsigned)i < n){
		NEEDBITS((unsigned)bl)
		j = (td = tl + ((unsigned)b & m))->b;
		DUMPBITS(j)
		j = td->v.n;
		if(j < 16)
			ll[i++] = l = j;
		else
			if(j == 16){
				NEEDBITS(2)
				j = 3 + ((unsigned)b & 3);
				DUMPBITS(2)
				if((unsigned)i + j > n)
					return 1;
				while(j--)
					ll[i++] = l;
			}
			else
				if(j == 17){
					NEEDBITS(3)
					j = 3 + ((unsigned)b & 7);
					DUMPBITS(3)
					if((unsigned)i + j > n)
						return 1;
					while(j--)
						ll[i++] = 0;
					l = 0;
				}
				else {
					NEEDBITS(7)
					j = 11 + ((unsigned)b & 0x7f);
					DUMPBITS(7)
					if((unsigned)i + j > n)
						return 1;
					while(j--)
						ll[i++] = 0;
					l = 0;
				}
	}
	huft_free(tl);

	bb = b;
	bk = k;
	bl = lbits;
	if((i = huft_build(ll,nl,257,cplens,cplext,&tl,&bl)) != 0){
		if(i == 1)
		huft_free(tl);
		return i;
	}
	bd = dbits;
	if((i = huft_build(ll + nl,nd,0,cpdist,cpdext,&td,&bd)) != 0){
		if(i == 1)
			huft_free(td);
		huft_free(tl);
		return i;
	}
	if(inflate_codes(tl,td,bl,bd))
		return 1;
	huft_free(tl);
	huft_free(td);
	return 0;
}

int inflate_block(int* e)
{
	unsigned t;
	ulong b;
	unsigned k;

	b = bb;
	k = bk;

	NEEDBITS(1)
	*e = (int)b & 1;
	DUMPBITS(1)

	NEEDBITS(2)
	t = (unsigned)b & 3;
	DUMPBITS(2)

	bb = b;
	bk = k;

	if(t == 2)
		return inflate_dynamic();
	if(t == 0)
		return inflate_stored();
	if(t == 1)
		return inflate_fixed();
	return 2;
}

int inflate_entry()
{
	int e;
	int r;
	unsigned h;

	wp = 0;
	bk = 0;
	bb = 0;

	h = 0;
	do {
		hufts = 0;
		if((r = inflate_block(&e)) != 0)
			return r;
		if(hufts > h)
			h = hufts;
	} while(!e);

	flush(wp);
	return 0;
}

void inflate()
{
	inflate_entry();
}

int ReadByte(ushort* x)
{
	return ReadMemoryByte(x);
}

