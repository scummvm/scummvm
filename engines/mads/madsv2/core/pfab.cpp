/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/textconsole.h"
#include "mads/madsv2/core/pfab.h"

namespace MADS {
namespace MADSV2 {

/*
 * pfab.cpp  --  LZ77 sliding-dictionary compressor/expander
 *
 * Ported from MicroProse "MPS Labs Graphic Library" assembly sources:
 *   pFABcomp.asm  (David McKibbin, September 5 1992)
 *   pFABexp0.asm  (file-to-file decompressor)
 *   pFABexp1.asm  (file-to-memory decompressor)
 *   pFABexp2.asm  (memory-to-memory decompressor)
 *
 * Copyright (c) 1992 MicroProse Software.  All Rights Reserved.
 * C port preserves all algorithmic behaviour of the original assembly.
 *
 * Algorithm notes
 * ---------------
 * Compressed stream format (PKWARE/LZexe hybrid, "FAB" magic):
 *
 *   Header : 'F','A','B', DICT          (4 bytes)
 *
 *   Each encoded unit consists of a Huffman packet.  A packet contains
 *   a 16-bit control word followed by up to 16 data items.  Each bit in
 *   the control word (LSB first) governs one data item:
 *
 *     1  -> LITERAL   : one literal byte follows.
 *     00 -> SHORT COPY: two Huffman bits (length 2-5) then one offset byte
 *                       (distance 1-255).
 *     01 -> LONG  COPY: one offset byte + one combined byte:
 *                         upper (16-DICT) bits = high offset bits
 *                         lower (DICT-4) bits  = length - 2   (2..17)
 *                       If that length field == (1<<(16-DICT))-1  a second
 *                       byte follows as the raw length (length = byte+1).
 *     01 + offset==0 + len==0  -> NORMALIZE segment fixup marker.
 *     01 + offset==0 + len==1  -> EXIT  (end of stream).
 *
 * Dictionary is a circular buffer of LEMPEL (4096) bytes.
 * Back-references are negative offsets into that buffer.
 */

#include "pfab.h"
#include <stddef.h>
#include <string.h>

 /* -------------------------------------------------------------------------
  * Compile-time constants (must match compressor and decompressor)
  * ---------------------------------------------------------------------- */
#define DICT      12                    /* log2 of dictionary size         */
#define ZIV       253                   /* max match length (bytes)        */
#define LEMPEL    (1 << DICT)           /* dictionary window  = 4096       */
#define NIL       (LEMPEL * 2)          /* sentinel / empty-table entry    */
#define HASH      (LEMPEL / 2)          /* hash table size (entries)       */

#define WBlen     2048                  /* compressor write-buffer size    */
#define RBlen     2048                  /* compressor read-buffer size     */

  /* =========================================================================
   *  I/O callback typedefs (match the original Pascal calling convention
   *  signatures translated to plain C function pointers)
   * ====================================================================== */
typedef word(*ReadFn) (char *buffer, word *size);
typedef word(*WriteFn)(char *buffer, word *size);


/* =========================================================================
 *  pFABcomp  --  LZ77 compressor  (PFABCOMP.ASM)
 *
 *  Compresses the data returned by read_buff into write_buff.
 *  work_buff must be at least pFABcomp(NULL,NULL,NULL,NULL,NULL) bytes.
 *  On success *dsize receives the compressed byte count and 0 is returned.
 * ====================================================================== */

 /* ----- compressor work state ------------------------------------------ */
struct CompWork {
	/* --- I/O ---                                                         */
	ReadFn  zread;
	WriteFn zwrite;

	unsigned  rb_ptr;               /* index into rbuff[]                  */
	unsigned  rb_cnt;               /* bytes remaining in rbuff            */

	unsigned  wb_ptr;               /* index into wbuff[]                  */
	unsigned  wb_cnt;               /* bytes remaining in wbuff            */

	unsigned long zsize;            /* total compressed bytes written      */

	/* --- LZ state ---                                                    */
	int       zlen;                 /* current Ziv length                  */
	int       qlen;                 /* match length                        */
	int       qoff;                 /* match offset (negative)             */
	int       norms;                /* bytes since last NORMALIZE          */
	int       delta;                /* counter for paraz                   */
	int       paraz;                /* extra paragraphs for EXPLODE        */

	/* --- Huffman packet ---                                              */
	unsigned  huffbit;              /* current bit position in acc         */
	unsigned  huffman;              /* accumulator (control word)          */
	unsigned  huffptr;              /* index into packet[]                 */
	byte packet[130];      /* packet buffer (2 + 128 data bytes)  */

	/* --- dictionary ---                                                  */
	byte lempel[LEMPEL];
	byte ziv[ZIV + 1];

	unsigned hash[LEMPEL + 1 + HASH];  /* collision + sentinel + key table */
	unsigned undo[LEMPEL + 1];

	/* --- I/O staging ---                                                 */
	word rwlen;
	byte  rbuff[RBlen];
	byte  wbuff[WBlen];
};

/**
 * Required size of work_buff.  Paragraph-align to match original.
 */
static unsigned comp_work_size(void) {
	return (unsigned)(sizeof(CompWork) + 15u);
}

/**
 * Obtain aligned pointer to CompWork inside work_buff.
 */
static CompWork *get_comp_work(char *work_buff) {
	intptr addr = (intptr)(byte *)work_buff;
	addr = (addr + 15u) & ~15u;
	return (CompWork *)(byte *)addr;
}

/*
 * ---- internal helpers -------------------------------------------------
 *
 * @param w	
 */

static void wb_out(CompWork *w) {
	unsigned cnt = w->wb_ptr;  // bytes written since last flush
	w->wb_ptr = 0;
	w->wb_cnt = WBlen;
	w->rwlen = (word)cnt;
	w->zsize += cnt;
	w->zwrite((char *)w->wbuff, &w->rwlen);
}

static void wb_put(CompWork *w, byte byte) {
	w->wbuff[w->wb_ptr++] = byte;
	if (--w->wb_cnt == 0)
		wb_out(w);
}

/**
 * Flush the current Huffman packet to the output stream.
 */
static void pkt_flush(CompWork *w) {
	// write packet[0..huffptr-1]: control word then data bytes
	w->packet[0] = (byte)(w->huffman & 0xFF);
	w->packet[1] = (byte)(w->huffman >> 8);

	unsigned end = w->huffptr;
	for (unsigned i = 0; i < end; i++)
		wb_put(w, w->packet[i]);

	w->huffbit = 1;
	w->huffman = 0;
	w->huffptr = 2;  // skip the two control-word bytes
}

/**
 * Append one Huffman control bit (0 or non-0).
 */
static void huff1(CompWork *w, int bit) {
	if (bit)
		w->huffman |= w->huffbit;
	w->huffbit <<= 1;
	if (w->huffbit == 0) {
		// control word is full (16 bits) - flush packet
		// The packet array already contains the data bytes at [2..ptr)
		// write everything out now
		byte tmp_ctrl0 = (byte)(w->huffman & 0xFF);
		byte tmp_ctrl1 = (byte)(w->huffman >> 8);
		unsigned save_ptr = w->huffptr;

		// emit control bytes then data bytes
		wb_put(w, tmp_ctrl0);
		wb_put(w, tmp_ctrl1);
		for (unsigned i = 2; i < save_ptr; i++)
			wb_put(w, w->packet[i]);

		w->huffbit = 1;
		w->huffman = 0;
		w->huffptr = 2;
	}
}

/**
 * Append one literal/copy data byte to the current packet buffer.
 */
static void push1(CompWork *w, byte byte) {
	w->packet[w->huffptr++] = byte;
}

/**
 * ---- rb_in: fill the read buffer --------------------------------------
 */
static unsigned rb_in(CompWork *w) {
	w->rb_ptr = 0;
	w->rwlen = (word)RBlen;
	unsigned got = w->zread((char *)w->rbuff, &w->rwlen);
	w->rb_cnt = got + 1;  // +1 so the main loop pre-decrements
	return got;
}

/* ---- dictionary link/unlink ------------------------------------------- */

/**
 * Hash(inword) = (word & (HASH-1)) + LEMPEL + 1
 * All indices into hash[]/undo[] are in units of unsigned (words).
 * In the assembly, indices are byte offsets and the arrays are word arrays,
 * hence the frequent shl/shr 1.  Here we use word indices throughout.
 *
 * @param w	
 * @param di	
 */
static void dict_link(CompWork *w, unsigned di) {
	unsigned dictWord = w->lempel[di] | ((unsigned)w->lempel[(di + 1) & (LEMPEL - 1)] << 8);
	unsigned key = (dictWord & (HASH - 1)) + LEMPEL + 1;  // key index
	unsigned di2 = di;  // word index = di

	unsigned bx = w->hash[key];
	w->hash[di2] = bx;
	w->hash[key] = di2;
	w->undo[bx] = di2;
	w->undo[di2] = key;
}

static void dict_unlink(CompWork *w, unsigned si) {
	unsigned bx = w->undo[si];
	if (bx != NIL) {
		w->hash[bx] = NIL;
		w->undo[si] = NIL;
	}
}

/**
 * ---- match: find the longest match at lempel[di] ----------------------
 */
static void match(CompWork *w, unsigned di) {
	// Compute hash key for the 2-byte word at lempel[di]
	unsigned inword = w->lempel[di]
		| ((unsigned)w->lempel[(di + 1) & (LEMPEL - 1)] << 8);
	unsigned key = (inword & (HASH - 1)) + LEMPEL + 1;

	int   best_len = 0;
	unsigned best_pos = 0;
	int   rem = ZIV - 1;  // chars still to match

	unsigned chain = w->hash[key];
	while (chain != NIL) {
		// compare lempel[chain+1 .. chain+ZIV-1] with lempel[di+1 ..]
		unsigned ms = (chain + 1) & (LEMPEL - 1);
		unsigned md = (di + 1) & (LEMPEL - 1);
		int matched = 0;
		while (matched < ZIV - 1 && w->lempel[ms] == w->lempel[md]) {
			ms = (ms + 1) & (LEMPEL - 1);
			md = (md + 1) & (LEMPEL - 1);
			matched++;
		}
		int this_len = matched + 1;  // +1 for the hashed first char

		if (matched == ZIV - 1) {
			// full ZIV match
			best_len = ZIV;
			best_pos = chain;
			break;
		}
		if (this_len > best_len) {
			best_len = this_len;
			best_pos = chain;
			rem = ZIV - 1 - matched;
		}
		chain = w->hash[chain];
	}

	w->qlen = best_len;

	if (best_len > 0) {
		// offset = di - best_pos (mod LEMPEL), stored negative
		int off = (int)di - (int)best_pos;
		off &= (LEMPEL - 1);
		w->qoff = -off;
	}

	// Also link di into the hash table (same as Match does at the end)
	unsigned di2 = di;
	unsigned bx = w->hash[key];
	unsigned di_hash = di2;
	w->hash[di_hash] = bx;
	w->hash[key] = di_hash;
	w->undo[bx] = di_hash;
	w->undo[di_hash] = key;
}

/**
 * ---- Fabrice: main LZ compression loop --------------------------------
 */
static unsigned long fabrice(CompWork *w) {
	unsigned si = 0;

	// Initialise I/O state
	w->rb_cnt = 1;  // force first read
	w->wb_ptr = 0;
	w->wb_cnt = WBlen;

	// Initialise dictionary
	memset(w->undo, 0xFF, sizeof(w->undo));  // NIL = 0xFFFF (LEMPEL*2)
	memset(w->hash, 0xFF, sizeof(w->hash));
	memset(w->lempel, 0, sizeof(w->lempel));
	memset(w->ziv, 0, sizeof(w->ziv));

	// Initialise Huffman packet
	w->huffbit = 1;
	w->huffman = 0;
	w->huffptr = 2;  // reserve two bytes for ctrl word

	w->norms = 0;
	w->delta = 0;
	w->paraz = 0;
	w->zsize = 0;

	// Write the 4-byte "FAB" header
	wb_put(w, 'F');
	wb_put(w, 'A');
	wb_put(w, 'B');
	wb_put(w, (byte)DICT);

	// Read first ZIV bytes into the top of the Lempel buffer
	unsigned di = (unsigned)(LEMPEL - ZIV);
	unsigned cx = ZIV;

	while (cx > 0) {
		if (--w->rb_cnt == 0) {
			if (rb_in(w) == 0)
				break;
		}
		w->lempel[di++] = w->rbuff[w->rb_ptr++];
		di &= (LEMPEL - 1);
		cx--;
	}

	unsigned read_so_far = ZIV - cx;
	if (read_so_far == 0)
		goto lz_exit;  // empty input

	w->zlen = (int)read_so_far;

	di = (unsigned)(LEMPEL - ZIV);  // encode pointer
	si = 0;  // end-of-ziv pointer

	match(w, di);

	// ---- main encoding loop ------------------------------------------
	for (;;) {
		// clip qlen to zlen
		if (w->qlen > w->zlen)
			w->qlen = w->zlen;

		int save_si = (int)si;
		int save_di = (int)di;

		int qlen = w->qlen;
		int qoff = w->qoff;

		if (qlen < 2 || (qlen == 2 && qoff >= -0x100)) {
			// LITERAL: control bit = 1, then data byte
			w->qlen = 1;
			huff1(w, 1);
			push1(w, w->lempel[di]);
		} else {
			huff1(w, 0);

			if (qlen <= 5 && qoff > -0x100) {
				// SHORT COPY: <00XY>, offset_byte  (length 2-5, dist 1-255)
				huff1(w, 0);
				int enc_len = qlen - 2;  // 0..3
				huff1(w, enc_len & 2);
				huff1(w, enc_len & 1);
				push1(w, (byte)qoff);  // qoff is negative; byte cast = low 8
			} else {
				// LONG COPY: <01>, vector_word
				huff1(w, 1);
				push1(w, (byte)qoff);  // low byte of offset

				byte hi = (byte)((unsigned)(-qoff) >> 8);
				int zb = 16 - DICT;
				hi = (byte)(hi << zb);

				int max_short_len = (1 << zb) + 1;
				if (qlen <= max_short_len) {
					hi |= (byte)(qlen - 2);
					push1(w, hi);
				} else {
					// LONG COPY with extra length byte
					push1(w, hi);
					push1(w, (byte)(qlen - 1));
				}
			}
		}

		// Update delta/paraz counters (track EXPLODE segment boundaries)
		w->delta -= w->qlen;
		while (w->delta < 0) {
			w->paraz++;
			w->delta += 16;
		}

		// Insert NORMALIZE marker every 40KB of output
		w->norms += w->qlen;
		if (w->norms >= 0xA000) {
			huff1(w, 0);
			huff1(w, 1);
			push1(w, 0);
			push1(w, 0);
			push1(w, 1);
			w->norms = 0;
		}

		di = (unsigned)save_di;
		si = (unsigned)save_si;

		// Advance the dictionary window by qlen positions
		int advance = w->qlen;
		while (advance-- > 0) {
			dict_link(w, di);
			dict_unlink(w, si);

			if (--w->rb_cnt == 0) {
				if (rb_in(w) == 0) {
					w->zlen--;
					goto skip_byte;
				}
			}
			{
				byte c = w->rbuff[w->rb_ptr++];
				w->lempel[si] = c;
				if (si < (unsigned)(ZIV - 1))
					w->ziv[si] = c;  // mirror first ZIV-1 chars
			}
skip_byte:
			si = (si + 1) & (LEMPEL - 1);
			di = (di + 1) & (LEMPEL - 1);
		}

		match(w, di);

		if (w->zlen <= 0)
			break;
	}

lz_exit:
	// Write EXIT marker: <01>, 0x00, 0x00, 0x00
	huff1(w, 0);
	huff1(w, 1);
	push1(w, 0);
	push1(w, 0);
	push1(w, 0);

	// Flush final packet and output buffer
	pkt_flush(w);
	if (w->wb_ptr > 0)
		wb_out(w);

	return w->zsize;
}

word pFABcomp(ReadFn read_buff, WriteFn write_buff, char *work_buff,
	word *type, word *dsize) {
	// If work_buff is NULL, return required buffer size
	if (!work_buff)
		return comp_work_size();

	CompWork *w = get_comp_work(work_buff);

	w->zread = read_buff;
	w->zwrite = write_buff;

	unsigned long csize = fabrice(w);

	// Return compressed size via dsize (two 16-bit words = 32-bit size)
	if (dsize) {
		((word *)dsize)[0] = (word)(csize & 0xFFFF);
		((word *)dsize)[1] = (word)(csize >> 16);
	}

	return 0;  // CMP_NO_ERROR
}


/* =========================================================================
 *  pFABexp common decompressor core
 *
 *  The three assembly EXP variants all include "pFABexp.inc", which is the
 *  actual decompressor state machine.  We implement it once here and call
 *  it from each of the three public wrappers with different I/O strategies.
 *
 *  Stream layout (produced by pFABcomp above):
 *    Bytes 0-3 : 'F','A','B', DICT
 *    Then packets:
 *      Each packet: [ctrl_lo][ctrl_hi] [data bytes...]
 *      Control word is 16 bits, consumed LSB-first.
 *      bit=1  -> literal: read next data byte, write to output.
 *      bit=0, next_bit=0 -> SHORT copy: 2 more bits = len-2, 1 byte = dist.
 *      bit=0, next_bit=1 -> LONG  copy: 1 byte low-offset, 1 byte packed:
 *                             high bits = high offset, low bits = len-2
 *                           if len==0 && off==0 -> NORMALIZE (skip)
 *                           if len==1 && off==0 -> EXIT
 *                           if low bits all set  -> read extra len byte
 * ====================================================================== */

 /* I/O context passed into the inner decoder.                             */
struct ExpIO {
	/* INPUT: one of two modes                                            */
	int   from_file;                    /* 1 = stream via read callback   */
	ReadFn rfun;                        /* file-source read callback      */
	const byte *rptr;          /* memory-source read pointer     */
	/* OUTPUT: one of two modes                                           */
	int   to_file;                      /* 1 = stream via write callback  */
	WriteFn wfun;                       /* file-dest  write callback      */
	byte *wptr;                /* memory-dest write pointer      */

	/* For file I/O we buffer reads / writes                              */
	byte rbuf[2048];
	unsigned      rbuf_pos;
	unsigned      rbuf_end;

	byte wbuf[4096];
	unsigned      wbuf_pos;

	unsigned long out_count;            /* total bytes decompressed       */
};

/*
 * ---- Buffered-read helpers -------------------------------------------
 */

static byte exp_getbyte(ExpIO *io) {
	if (io->from_file) {
		if (io->rbuf_pos >= io->rbuf_end) {
			word sz = 2048;
			io->rfun((char *)io->rbuf, &sz);
			io->rbuf_end = sz;
			io->rbuf_pos = 0;
		}
		return (io->rbuf_pos < io->rbuf_end) ? io->rbuf[io->rbuf_pos++] : 0;
	} else {
		return *io->rptr++;
	}
}

static unsigned exp_getword(ExpIO *io) {
	unsigned lo = exp_getbyte(io);
	unsigned hi = exp_getbyte(io);
	return lo | (hi << 8);
}

/*
 * ---- Buffered-write helpers ------------------------------------------
 */

static void exp_flush_wbuf(ExpIO *io, int force_all) {
	if (!io->to_file || io->wbuf_pos == 0)
		return;

	byte *base = io->wbuf;
	unsigned       len = force_all ? io->wbuf_pos
		: (io->wbuf_pos & ~(4096u - 1u));
	if (len == 0)
		return;

	word sz = (word)len;
	io->wfun((char *)base, &sz);
	io->out_count += len;

	// Shift remaining bytes to front
	unsigned remain = io->wbuf_pos - len;
	if (remain)
		memmove(base, base + len, remain);
	io->wbuf_pos = remain;
}

static void exp_putbyte(ExpIO *io, byte b) {
	if (io->to_file) {
		io->wbuf[io->wbuf_pos++] = b;
		if (io->wbuf_pos == 4096)
			exp_flush_wbuf(io, 1);
	} else {
		*io->wptr++ = b;
		io->out_count++;
	}
}

/**
 * ---- read a byte from the output window (back-reference) -------------
 */
static byte exp_readback(ExpIO *io, int dist, unsigned long total_out) {
	// dist = 1..LEMPEL (positive, 1 = most recent byte written)
	if (!io->to_file) {
		// Simple: just look behind in the output buffer
		return *(io->wptr - dist);
	} else {
		// For file-to-file we maintain a sliding window in wbuf.
		// wbuf is used as circular output; wbuf_pos is the current head.
		unsigned idx = (io->wbuf_pos + 4096u - (unsigned)dist) & (4096u - 1u);
		return io->wbuf[idx];
	}
}

/**
 * ---- Core decompressor -----------------------------------------------
 */
static unsigned long fab_explode(ExpIO *io) {
	io->out_count = 0;

	// ---- Verify FAB header ------------------------------------------
	if (exp_getbyte(io) != 'F') return 0;
	if (exp_getbyte(io) != 'A') return 0;
	if (exp_getbyte(io) != 'B') return 0;
	byte shiftVal = exp_getbyte(io);  // dict bits, e.g. 12

	// Derive the same masks FabDecompressor uses:
	//   copyOfsShift = 16 - shiftVal          (e.g. 4)
	//   copyOfsMask  = 0xFF << (shiftVal - 8) (e.g. 0xF0) -- sign-extends
	//                                          the high offset nibble
	//   copyLenMask  = (1 << copyOfsShift) - 1 (e.g. 0x0F)
	byte copyOfsShift = (byte)(16 - shiftVal);
	byte copyOfsMask = (byte)(0xFF << (shiftVal - 8));
	byte copyLenMask = (byte)((1 << copyOfsShift) - 1);
	byte copyLen;
	unsigned long copyOfs;

	// Setup initial control bits word
	uint ctrl = exp_getword(io);
	int  cbits = 16;

	auto get_ctrl_bit = [&]() -> int {
		if (--cbits == 0) {
			ctrl = (exp_getword(io) << 1) | (ctrl & 1);
			cbits = 16;
		}

		int bit = (int)(ctrl & 1);
		ctrl >>= 1;
		return bit;
	};

	// ---- Decode loop -------------------------------------------------
	for (;;) {
		int bit0 = get_ctrl_bit();

		if (bit0 == 1) {
			// LITERAL: copy one byte straight through
			byte c = exp_getbyte(io);
			exp_putbyte(io, c);
			continue;
		}

		// bit0 == 0
		int bit1 = get_ctrl_bit();

		if (bit1 == 0) {
			// Short copy
			int lb = get_ctrl_bit();
			int la = get_ctrl_bit();
			copyLen = ((lb << 1) | la) + 2;
			copyOfs = exp_getbyte(io) | 0xFFFFFF00;
		} else {
			// Long copy
			int lb = exp_getbyte(io);
			int la = exp_getbyte(io);
			copyOfs = (((la >> copyOfsShift) | copyOfsMask) << 8) | lb;
			copyLen = la & copyLenMask;

			if (copyLen == 0) {
				copyLen = exp_getbyte(io);
				if (copyLen == 0)
					// End of decompression
					break;
				else if (copyLen == 1)
					continue;
				else
					copyLen++;
			} else {
				copyLen += 2;
			}
			copyOfs |= 0xFFFF0000;
		}

		int dist = ABS((signed int)copyOfs);
		while (copyLen-- > 0) {
			byte c = exp_readback(io, dist, io->out_count);
			exp_putbyte(io, c);
		}
	}

	// Flush any remaining output                                         */
	if (io->to_file)
		exp_flush_wbuf(io, 1);

	return io->out_count;
}


/* =========================================================================
 *  pFABexp0  --  file-to-file decompressor  (PFABEXP0.ASM)
 *
 *  Reads compressed data via read_buff, writes decompressed data via
 *  write_buff.  Returns 0 on success; the 32-bit decompressed byte count
 *  is stored as two unsigned shorts at work_buff[0..3].
 * ====================================================================== */

 /* Minimum work_buff size for pFABexp0 (original needed ~14364 bytes).    */
#define EXP0_WORK_SIZE  16384u

word pFABexp0(ReadFn read_buff, WriteFn write_buff, char *work_buff) {
	if (!work_buff)
		return EXP0_WORK_SIZE;

	ExpIO io;
	memset(&io, 0, sizeof(io));

	io.from_file = 1;
	io.rfun = read_buff;
	io.to_file = 1;
	io.wfun = write_buff;

	unsigned long out_len = fab_explode(&io);

	// Return decompressed size in work_buff[0..3]
	((word *)work_buff)[0] = (word)(out_len & 0xFFFF);
	((word *)work_buff)[1] = (word)(out_len >> 16);

	return 0;  // CMP_NO_ERROR
}


/* =========================================================================
 *  pFABexp1  --  file-to-memory decompressor  (PFABEXP1.ASM)
 *
 *  Reads compressed data via read_buff, writes decompressed data starting
 *  at write_buf.  Returns 0 on success; the 32-bit decompressed byte count
 *  is stored as two unsigned shorts at work_buff[0..3].
 * ====================================================================== */

 /* Minimum work_buff size for pFABexp1 (original needed ~2072 bytes).     */
#define EXP1_WORK_SIZE  4096u

word pFABexp1(ReadFn read_buff, char *write_buf, char *work_buff) {
	if (!work_buff)
		return EXP1_WORK_SIZE;

	ExpIO io;
	memset(&io, 0, sizeof(io));

	io.from_file = 1;
	io.rfun = read_buff;
	io.to_file = 0;
	io.wptr = (byte *)write_buf;

	unsigned long out_len = fab_explode(&io);

	// Return decompressed size in work_buff[0..3]
	((word *)work_buff)[0] = (word)(out_len & 0xFFFF);
	((word *)work_buff)[1] = (word)(out_len >> 16);

	return 0;
}


word pFABexp2(byte *read_buf, byte *write_buf, char *work_buff) {
	if (!work_buff)
		return 4u;  // original returned 4

	ExpIO io;
	memset(&io, 0, sizeof(io));

	io.from_file = 0;
	io.rptr = read_buf;
	io.to_file = 0;
	io.wptr = write_buf;

	unsigned long out_len = fab_explode(&io);

	// Return decompressed size in work_buff[0..3]
	((word *)work_buff)[0] = (word)(out_len & 0xFFFF);
	((word *)work_buff)[1] = (word)(out_len >> 16);

	return 0;
}

} // namespace MADSV2
} // namespace MADS
