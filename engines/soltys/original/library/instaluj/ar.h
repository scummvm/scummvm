/***********************************************************
	ar.h
***********************************************************/
#include <stdio.h>
#include <limits.h>
typedef unsigned char  uchar;   //  8 bits or more
typedef unsigned int   uint;    // 16 bits or more
typedef unsigned short ushort;  // 16 bits or more
typedef unsigned long  ulong;   // 32 bits or more

// ar.c

extern int unpackable;
extern ulong origsize, compsize;
void	Drop	(const char *msg);



// io.c

#define INIT_CRC  0  /* CCITT: 0xFFFF */
extern uint crc, bitbuf;
#define BITBUFSIZ (CHAR_BIT * sizeof bitbuf)

//void error(char *fmt, ...);
void make_crctable(void);
void fillbuf(int n);
uint getbits(int n);
// void putbit(int bit);
void putbits(int n, uint x);
void init_getbits(void);
void init_putbits(void);

// encode.c and decode.c

#define DICBIT    13    // 12(-lh4-) or 13(-lh5-)
#define DICSIZ (1U << DICBIT)
#define MATCHBIT   8    // bits for MAXMATCH - THRESHOLD
#define MAXMATCH 256    // formerly F (not more than UCHAR_MAX + 1)
#define THRESHOLD  3    // choose optimal value
#define PERC_FLAG 0x8000U

void decode_start(void);
void decode(uint count, uchar text[]);

// huf.c

#define NC (UCHAR_MAX + MAXMATCH + 2 - THRESHOLD)
	// alphabet = {0, 1, 2, ..., NC - 1}
#define CBIT 9  // $\lfloor \log_2 NC \rfloor + 1$
#define CODE_BIT  16  // codeword length

extern ushort left[], right[];

void huf_decode_start(void);
uint decode_c(void);
uint decode_p(void);

// maketbl.c

void make_table(int nchar, uchar bitlen[],
				int tablebits, ushort table[]);

// maketree.c

int make_tree(int nparm, ushort freqparm[],
				uchar lenparm[], ushort codeparm[]);
