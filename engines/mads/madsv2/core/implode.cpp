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
#include "mads/madsv2/core/implode.h"

namespace MADS {
namespace MADSV2 {

/*
 * implode_explode.c
 *
 * Translated from IMPLODE.ASM / EXPLODE.ASM
 *   Original author : David McKibbin, MPS Labs / MicroProse Software (1992)
 *   Algorithm       : LZexe-style LZ77 with Huffman control bits (FAB/Fabrice format)
 *
 * Translation notes:
 *   - All segment-register / far-pointer mechanics removed (flat memory model).
 *   - DS:SI / ES:DI read/write patterns replaced with pointer arithmetic.
 *   - The WORK segment layout is preserved as struct fields in the same order
 *     so buffer-size requirements are unchanged.
 *   - 16-bit wraparound arithmetic preserved where it matters (hash indices,
 *     Qlen/Qoff calculations, GetHuff bit pump).
 */

#include <string.h>
#include <stdint.h>
#include "implode.h"    /* extern declarations + CMP_* defines */

typedef unsigned char  byte;
typedef unsigned short word;

/* =========================================================================
 *  EXPLODE
 *
 *  Memory layout (mirrors the assembly WORK segment, total 14348 bytes):
 *
 *    [  0..  3]  Zread  (unused in flat C; callback pointer held in struct)
 *    [  4..  7]  Zwrite (unused in flat C)
 *    [  8..  9]  RWlen  (unused in flat C)
 *    [ 10.. 11]  guard word (1 extra word before RBuff)
 *    [ 12..2059] RBuff  (RBlen=2048 bytes)   RBend = RBuff+RBlen-1
 *    [2060..10251] Lempel (8192 bytes)
 *    [10252..14347] WBuff (WBlen=4096 bytes)  WBend = WBuff+WBlen-1
 *
 *  SI (read ptr) is an index into RBuff[0..RBlen-1] (with guard at [-1]).
 *  DI (write ptr) is an index into the combined Lempel+WBuff flat array,
 *     starting at 0 (=Lempel base); WBuff starts at index 8192.
 *
 * ========================================================================= */

#define EXP_RBLEN  2048
#define EXP_WBLEN  4096
#define EXP_LEMPEL 8192

 /* explode_RBin:
  *   Guard byte trick: before overwriting RBuff with new data, save the byte
  *   that was at rb[EXP_RBLEN] (one past the last valid byte) into rb[-1]
  *   (guard slot).  This lets GetWord safely read a 16-bit value when SI is
  *   positioned at the very last byte of the buffer - the high byte comes from
  *   the guard slot after the new fill lands in rb[0..EXP_RBLEN-1].
  *   In C we allocate rb[EXP_RBLEN+1] and use rb[0] as the guard, rb[1..EXP_RBLEN]
  *   as the data, keeping si pointing into [1..EXP_RBLEN] (mirrors assembly SI
  *   pointing into RBuff[0..RBlen-1], with the guard word sitting just below).
  */
typedef struct {
	word(*read_buff) (char *buf, word *size);
	word(*write_buff)(char *buf, word *size);

	/* Read buffer: rb[0]=guard, rb[1..EXP_RBLEN]=data */
	byte   rb[EXP_RBLEN + 1];
	int    si;          /* current read index into rb[], range [1..EXP_RBLEN] */
	/* si <= EXP_RBLEN  => valid byte at rb[si]  (mirrors si <= RBend) */

	/* Huffman bit pump (registers BP and DX in the assembly) */
	word   bp;          /* bit accumulator */
	int    dx;          /* bits remaining before next word load */

	/* Combined Lempel + WBuff flat array.
	 * flat[0..8191]      = Lempel (history window)
	 * flat[8192..12287]  = WBuff  (output staging buffer)
	 * DI is an index into flat[], starting at EXP_LEMPEL (= WBuff base).
	 * Back-references: src = flat[di + bx] where bx is a negative int16.
	 * Minimum src = EXP_LEMPEL - EXP_LEMPEL = 0.  No underflow possible.
	 */
	byte   flat[EXP_LEMPEL + EXP_WBLEN];
	int    di;          /* write index into flat[], starts at EXP_LEMPEL */
} ExpState;

/**
 * Fill the read buffer.
 * Before reading: save rb[si] (which is rb[EXP_RBLEN+1] after last refill,
 * i.e. one past end) into rb[0] (the guard slot).  Then reset si to 1.
 *
 * @param s		State
 */
static void explode_RBin(ExpState *s) {
	// Save the byte just past the current buffer end into the guard slot.
	// In the assembly this is: mov al,[si]; sub si,RBlen; mov [si],al
	// which saves rb[EXP_RBLEN] to rb[0] (= guard word low byte).
	s->rb[0] = s->rb[s->si];  // save guard byte
	s->si = 1;  // reset to start of data region

	word size = EXP_RBLEN;
	s->read_buff((char *)(s->rb + 1), &size);
	// If fewer bytes were read than requested, the remainder of rb[]
	// retains its previous content (harmless for a decompressor consuming
	// exactly the right number of bytes from a well-formed stream).
}

/**
 * Flush the write (output) buffer.
 * Writes flat[EXP_LEMPEL .. di-1] to the output callback, then performs the
 * Lempel fixup: copies flat[EXP_LEMPEL-4096 .. EXP_LEMPEL-1] (the second half
 * of the Lempel window) to flat[0 .. 4095] (the first half), then resets di.
 * This preserves the last 4096 bytes of history so back-references up to
 * -8192 bytes remain valid after the reset.  The fixup only happens when a
 * full WBlen buffer was flushed (matching the assembly's "cmp RWlen,WBlen").
 *
 * @param s		State
 */
static void explode_WBout(ExpState *s) {
	int bytes = s->di - EXP_LEMPEL;  // bytes currently in WBuff
	word size = (word)bytes;
	s->write_buff((char *)(s->flat + EXP_LEMPEL), &size);

	if (bytes == EXP_WBLEN) {
		// Lempel fixup: copy second half of Lempel to first half.
		// Assembly: lea si, WBuff-4096 (= Lempel+4096)
		//           lea di, WBuff-8192 (= Lempel+0)
		//           mov cx, 4096; rep movsw  (copies 8192 bytes)
		memmove(s->flat, s->flat + EXP_LEMPEL - 4096, 4096);
	}

	s->di = EXP_LEMPEL;  // reset write pointer to WBuff base
}

/**
 * If si > EXP_RBLEN (= RBend+1, one past last byte), refill.
 * Then return rb[si++].
 *
 * @param s		State
 * @return	Next byte
 */
static inline byte exp_get_byte(ExpState *s) {
	if (s->si > EXP_RBLEN)
		explode_RBin(s);
	return s->rb[s->si++];
}

/**
 * If si >= EXP_RBLEN (= RBend, only 1 byte left), refill first.
 * The guard byte at rb[0] ensures the second byte of the word is available
 * even when si pointed to the very last slot before the refill.
 *
 * @param s		State
 * @return	Next word
 */
static inline word exp_get_word(ExpState *s) {
	if (s->si >= EXP_RBLEN)
		explode_RBin(s);
	word val = (word)s->rb[s->si] | ((word)s->rb[s->si + 1] << 8);
	s->si += 2;
	return val;
}

/**
 * Extract one bit from the Huffman bit pump.
 * Returns the bit in CF position (0 or 1).
 *
 * We get the saved carry bit as the 17th bit extracted, giving seamless
 * 16-bit-boundary crossing.  Verified by simulation to produce bits
 * in LSB-first order, one word at a time.
 *
 * @param s		State
 * @return		Next bit
 */
static inline int exp_get_huff(ExpState *s) {
	int bit;

	s->dx--;
	if (s->dx == 0) {
		word ax = exp_get_word(s);
		s->dx = 16;
		// shr bp,1 -> CF = bp & 1
		int carry = s->bp & 1;
		// mov bp, ax then rcl bp,1 -> bp = (ax<<1)|carry, new CF = ax>>15
		s->bp = (word)((ax << 1) | carry);
		// fall through: rcr bp,1 below extracts bp[0] = carry as current bit
	}

	// rcr bp,1: CF = bp[0], bp = (bp >> 1) | (CF_in << 15)
	// CF_in at this point is the CF left by the last operation above (or the
	// previous iteration's rcr).  In the assembly CF is a real flag and
	// carries implicitly; here we must track it.  But note: the ONLY place
	// CF matters going into rcr is on the reload path (where it was set by
	// rcl), and on the non-reload path CF is not used (the rcr just rotates
	// bit15 in, but since we never inspect bp's high bits, it doesn't matter
	// what rotates in).  We use 0 for CF_in on non-reload iterations; the
	// high bit of bp is never examined by the decoder.
	bit = s->bp & 1;
	s->bp >>= 1;  // bp >> 1, high bit = 0 (safe: only bit0 is inspected)
	return bit;
}

/**
 * If di > WBend (= EXP_LEMPEL + EXP_WBLEN - 1), flush first.
 *
 * @param s		State
 * @param val	Byte
 */
static inline void exp_put_byte(ExpState *s, byte val) {
	if (s->di > EXP_LEMPEL + EXP_WBLEN - 1)
		explode_WBout(s);
	s->flat[s->di++] = val;
}

/**
 * LZ77 decompressor
 *
 * Reads compressed data via read_buff, writes decompressed data via write_buff.
 * work_buff must point to at least 14364 bytes of writable memory (ignored
 * in the flat C port; state lives on the stack via ExpState).
 */
word explode(
	word(*read_buff) (char *buffer, word *size),
	word(*write_buff)(char *buffer, word *size),
	char *work_buff) {
	(void)work_buff;

	ExpState s;
	memset(&s, 0, sizeof s);
	s.read_buff = read_buff;
	s.write_buff = write_buff;

	// Force immediate refill on first GetByte/GetWord.
	// Assembly: lea si, RBuff; add si, RBlen  (si = one past end of RBuff)
	s.si = EXP_RBLEN + 1;

	// ES:DI = WBuff base (flat index EXP_LEMPEL)
	s.di = EXP_LEMPEL;

	// Prime the Huffman pump: GetWord, bp = ax, dx = 16
	s.bp = exp_get_word(&s);
	s.dx = 16;

	// FABin: main decode loop
	//
	// GetHuff returns the next control bit in CF (carry).
	// Bit=1 => literal byte follows.
	// Bit=0 => back-reference.
	for (;;) {
		int bit = exp_get_huff(&s);

		if (bit) {
			// Literal: GetByte then PutByte
			byte al = exp_get_byte(&s);
			exp_put_byte(&s, al);
			continue;
		}

		// Back-reference.  Second control bit selects near (<00>) or far (<01>).
		int cx = 0;
		bit = exp_get_huff(&s);

		if (!bit) {
			// Code <00XY>: near copy.
			// Two more Huffman bits give XY (2-bit length field).
			// Length = XY + 2, range [2..5].
			// One raw byte gives the negative offset (BH=-1, BL=byte => BX=0xFF00|byte).
			//
			// Assembly:
			//   GetHuff; rcl cx,1   (cx = ...X)
			//   GetHuff; rcl cx,1   (cx = ...XY)
			//   inc cx; inc cx      (cx = XY + 2)
			//   GetByte
			//   mov bh,-1; mov bl,al   (bx = 0xFF00 | al)
			int x = exp_get_huff(&s);
			cx = (cx << 1) | x;
			int y = exp_get_huff(&s);
			cx = (cx << 1) | y;
			cx += 2;  // length [2..5]

			byte al = exp_get_byte(&s);
			int bx = (int)(int16_t)(0xFF00 | (word)al);  // signed 16-bit: [-255..-1]

			// LP3: copy cx bytes from flat[di + bx] to flat[di], advancing di.
			// Overlapping copies are intentional (run-length expansion).
			// Assembly: mov al, es:[bx+di]; PutByte; loop LP3
			while (cx-- > 0) {
				// di+bx: since bx is negative and di>=EXP_LEMPEL, result >= 0
				byte b = s.flat[s.di + bx];
				exp_put_byte(&s, b);
			}
		} else {
			// Code <01>: far copy (or special sentinel).
			// One raw word encodes offset and length:
			//   BL = low byte of word  = offset[7:0]
			//   BH = high byte, after processing:
			//     shr bh,3; or bh,0E0h  => offset[12:8] packed into bh
			//   AH & 7 = length field (original high byte before shr):
			//     0        => read aux byte (exit/normalize/long count)
			//     1..7     => length = field + 2
			//
			// Signed offset reconstruction:
			//   bx = (int16_t)( (((ah >> 3) | 0xE0) << 8) | al )
			//   Range: [-8192..-1]
			word ax = exp_get_word(&s);
			byte ah = (byte)(ax >> 8);
			byte al = (byte)(ax & 0xFF);

			// Reconstruct BH: shr bh,3 then or bh,0E0h
			byte bh = (byte)((ah >> 3) | 0xE0);
			// BX as signed 16-bit
			int bx = (int)(int16_t)((word)((word)bh << 8) | al);

			// Length field = original AH & 7 (before the shr)
			int len_field = ah & 7;

			if (len_field != 0) {
				// count = len_field + 2.
				// Assembly: mov cl, ah (ah already anded to 7); inc cx; inc cx
				cx = len_field + 2;

				// LP3 copy
				while (cx-- > 0) {
					byte b = s.flat[s.di + bx];
					exp_put_byte(&s, b);
				}
			} else {
				// LP3a: aux byte determines action.
				// 0 => EXIT
				// 1 => NORMALIZE (no-op in flat model; just continue)
				// n => count = n + 1, then LP3 copy
				byte aux = exp_get_byte(&s);

				if (aux == 0) {
					// EXIT sentinel
					break;
				} else if (aux == 1) {
					// LP4: NORMALIZE.  In the original this re-normalised
					// segment:offset pointers for the decompressor's internal
					// buffers.  In the flat C model this is a no-op.
					continue;
				} else {
					cx = (int)aux + 1;
					while (cx-- > 0) {
						byte b = s.flat[s.di + bx];
						exp_put_byte(&s, b);
					}
				}
			}
		}
	}

	// Exit: flush any remaining output bytes
	explode_WBout(&s);

	return CMP_NO_ERROR;
}


/* =========================================================================
 *  IMPLODE
 *
 *  Memory layout (mirrors the assembly WORK segment, total ~53670 bytes):
 *
 *    Zread/Zwrite  : callback pointers (held in struct, not in the flat buf)
 *    RBptr/RBcnt   : read buffer management
 *    WBptr/WBcnt   : write buffer management
 *    Zlen          : bytes of valid lookahead remaining
 *    Qlen/Qoff     : current match length and offset
 *    Norms         : bytes since last NORMALIZE code
 *    Delta/ParaZ   : segment-fixup bookkeeping (preserved but output only)
 *    Huffptr/bit/man: Huffman packet state
 *    Lempel[8192]  : sliding dictionary window
 *    Ziv[ZIV+1]    : shadow of Lempel[0..ZIV-1] for wraparound comparisons
 *    Hash[]        : (LEMPEL+1+HASH)*2 bytes, combined collision+key table
 *    UnDo[]        : (LEMPEL+1)*2 bytes, reverse-thread for unlinking
 *    RBuff[2048]   : raw input buffer
 *    WBuff[2048]   : raw output buffer
 *
 * ========================================================================= */

#define IMP_ZIV    253
#define IMP_LEMPEL 8192
#define IMP_NIL    (2*IMP_LEMPEL)     /* = 16384 = 0x4000, sentinel value */
#define IMP_HASH   4096
#define IMP_RBLEN  2048
#define IMP_WBLEN  2048

 /* Packet buffer: Huffman word (2 bytes) + up to 128 data bytes */
#define IMP_PKTMAX (2 + 128)

typedef struct {
	word(*read_buff) (char *buf, word *size);
	word(*write_buff)(char *buf, word *size);

	/* Read buffer state */
	byte   rb[IMP_RBLEN];
	byte *rb_ptr;          /* next byte to read from rb[] */
	word   rb_cnt;          /* bytes remaining incl. current (mirrors RBcnt) */

	/* Write buffer state */
	byte   wb[IMP_WBLEN];
	byte *wb_ptr;          /* next free slot in wb[] */
	word   wb_cnt;          /* free bytes remaining in wb[] */

	/* LZ state variables */
	word   zlen;            /* valid lookahead bytes */
	word   qlen;            /* match length */
	int16_t qoff;           /* match offset (negative) */
	word   norms;           /* bytes since last normalize code */
	int16_t delta;          /* counter for paraz */
	word   paraz;           /* extra paragraphs needed by explode */

	/* Huffman packet state */
	byte *huff_ptr;        /* next data byte slot in packet[] */
	word   huff_bit;        /* current bit mask (1, 2, 4, ..., 0x8000) */
	word   huff_man;        /* current Huffman accumulator word */
	byte   packet[IMP_PKTMAX]; /* packet buffer: [0..1]=huff_man, [2..]=data */

	/* Sliding dictionary */
	byte   lempel[IMP_LEMPEL + IMP_ZIV + 1]; /* Lempel[0..8191] + Ziv[0..253] */

	/* Hash table: (IMP_LEMPEL+1+IMP_HASH) word entries.
	 * Indexed by BYTE OFFSET (= position*2).
	 * [0 .. IMP_LEMPEL-1]         : collision chain slots  (one per Lempel pos)
	 * [IMP_LEMPEL]                : sentinel slot  (always holds NIL)
	 * [IMP_LEMPEL+1..IMP_LEMPEL+IMP_HASH] : hash key roots
	 * UnDo[0..IMP_LEMPEL]: reverse-thread, indexed by byte offset.
	 *
	 * Both tables store BYTE OFFSETS (position<<1) as values, with NIL=0x4000
	 * marking empty slots.
	 */
	word   hash[IMP_LEMPEL + 1 + IMP_HASH];
	word   undo[IMP_LEMPEL + 1];
} ImpState;

/* ---- I/O helpers ---- */

/**
 * Fill the read buffer.
 * Assembly increments RBcnt by (bytes_read+1) then decrements before each use,
 * so RBcnt==1 means "need refill on next access".
 *
 * @param s		State
 * @return	bytes read (0 = EOF).
 */
static word imp_rb_refill(ImpState *s) {
	word size = IMP_RBLEN;
	s->read_buff((char *)s->rb, &size);
	s->rb_ptr = s->rb;
	s->rb_cnt = size + 1;  // +1: assembly sets RBcnt = bytes+1 (see RBin)
	return size;
}

/**
 * Read one byte from the input.  Returns -1 (as int) on EOF.
 * Assembly pattern (inside Fabrice loop):
 * dec RBcnt; jnz Gin; call implode_RBin; or ax,ax; jz Zin; jmp @B
 * Gin: mov bx,RBptr; inc RBptr; mov al,[bx]
 * RBcnt starts at 1 after init; first dec makes it 0, triggering refill.
 * After refill: RBcnt = bytes_read + 1.
 *
 * @param s		State
 * @return	Next byte
 */
static int imp_get_byte(ImpState *s) {
	for (;;) {
		s->rb_cnt--;
		if (s->rb_cnt != 0)
			return *s->rb_ptr++;
		// RBcnt hit 0: refill
		word got = imp_rb_refill(s);
		if (got == 0)
			return -1;  // EOF
		// rb_cnt = got + 1, loop back to decrement
	}
}

/**
 * Flush the write (output) buffer.
 * Resets wb_ptr and wb_cnt.  Assembly version:
 * WBcnt = WBlen, compute bytes = WBptr - &WBuff, reset WBptr = &WBuff
 *
 * @param s		State
 */
static void imp_wb_flush(ImpState *s) {
	word bytes = (word)(s->wb_ptr - s->wb);
	s->wb_ptr = s->wb;
	s->wb_cnt = IMP_WBLEN;
	if (bytes > 0)
		s->write_buff((char *)s->wb, &bytes);
}

/**
 * Write one byte to the output buffer, flushing when full.
 *
 * @param s		State
 * @param val	Byte
 */
static void imp_wb_put(ImpState *s, byte val) {
	*s->wb_ptr++ = val;
	s->wb_cnt--;
	if (s->wb_cnt == 0)
		imp_wb_flush(s);
}

/* ---- Packet / Huffman output ---- */

/**
 * Flush the current Huffman packet to the write buffer and reset.
 * Assembly walks si from offset(Huffman) to Huffptr, writing each byte.
 * The packet layout is: huff_man word (2 bytes LE) followed by data bytes.
 * After flush: Huffbit=1, Huffman=0, Huffptr = &Huffman+2 (start of data).
 *
 * @param s		State
 */
static void imp_packet(ImpState *s) {
	// Store the current Huffman accumulator into packet[0..1] (little-endian).
	// Note: packet[] was already reset to 0 at start; partial huff_man is
	// whatever was accumulated before Huffbit overflowed.
	s->packet[0] = (byte)(s->huff_man & 0xFF);
	s->packet[1] = (byte)(s->huff_man >> 8);

	// Assembly: inc Delta at start of Packet
	s->delta++;

	// Flush all bytes from packet[0] to huff_ptr-1
	byte *p = s->packet;
	while (p < s->huff_ptr)
		imp_wb_put(s, *p++);

	// Reset packet state
	s->huff_bit = 1;
	s->huff_man = 0;
	s->packet[0] = 0;
	s->packet[1] = 0;
	s->huff_ptr = s->packet + 2;  // data starts after the 2-byte Huffman word
}

/**
 * Emit one Huffman control bit.
 * Assembly:
 * or al,al; jz @F
 * mov ax,Huffbit; or Huffman,ax    (set bit if val != 0)
 * @@: shl Huffbit,1; jnz @F; call Packet   (flush when bit overflows 16-bit)
 *
 * @param s		State
 * @param val	Value
 */
static void imp_huff1(ImpState *s, int val) {
	if (val)
		s->huff_man |= s->huff_bit;
	s->huff_bit <<= 1;
	if (s->huff_bit == 0) {
		// Huffbit has shifted out of 16-bit range: flush packet
		imp_packet(s);
	}
}

/**
 * Append one raw data byte to the current packet buffer.
 * Assembly: mov di,Huffptr; inc Huffptr; mov [di],al
 *
 * @param s		State
 * @param val	Value
 */
static void imp_push1(ImpState *s, byte val) {
	*s->huff_ptr++ = val;
}

/* ---- Dictionary management ---- */

/**
 * Hash key computation.
 * Assembly (in Link and Match):
 * mov si, word ptr Lempel[di]   ; 16-bit word at lempel[di]
 * and si, HASH-1                ; mask to [0..IMP_HASH-1]
 * add si, LEMPEL+1              ; shift into key-table region
 * shl si, 1                     ; convert to byte offset
 * Result is a BYTE OFFSET into hash[] pointing at the key root slot.
 * In C we use a WORD index = si>>1 into the hash[] word array.
 *
 * @param s		State
 * @param pos	
 * @return 
 */
static inline int imp_hash_key_idx(ImpState *s, int pos) {
	word w = (word)s->lempel[pos] | ((word)s->lempel[pos + 1] << 8);
	return (int)(((w & (IMP_HASH - 1)) + IMP_LEMPEL + 1));  // word index
}

/**
 * Insert lempel[di] at the root of its hash chain.
 * Assembly (byte-offset based):
 * si = key byte offset  (= hash_key_idx * 2)
 * di2 = di * 2
 * bx = Hash[si>>1]       ; old root byte-offset
 * Hash[di2>>1] = bx      ; di's next = old root
 * Hash[si>>1]  = di2     ; new root  = di
 * UnDo[bx>>1]  = di2     ; old root back-ptr = di
 * UnDo[di2>>1] = si      ; di back-ptr = key slot
 * All values stored are BYTE OFFSETS (position << 1), with NIL = 0x4000.
 * We store them directly as word values in hash[]/undo[] word arrays,
 * using word-index = byte-offset >> 1 to access them.
 *
 * @param s		State
 * @param di	Value
 */
static void imp_link(ImpState *s, int di) {
	int si_idx = imp_hash_key_idx(s, di);  // word index of key slot
	int di2 = di * 2;  // byte offset of di's slot
	int si2 = si_idx * 2;  // byte offset of key slot

	word bx = s->hash[si_idx];  // old root (byte offset)

	s->hash[di2 / 2] = bx;  // di's next = old root
	s->hash[si_idx] = (word)di2;  // new root = di
	s->undo[bx / 2] = (word)di2;  // old root back-ptr = di (byte off)
	s->undo[di2 / 2] = (word)si2;  // di's back-ptr = key slot (byte off)
}

/**
 * Remove lempel[si] from its hash chain.
 * Assembly:
 * si2 = si << 1
 * bx = UnDo[si2>>1]          ; the slot that currently holds si2
 * if bx != NIL:
 * Hash[bx>>1] = NIL        ; remove si2 from that slot
 * UnDo[si2>>1] = NIL
 *
 * @param s		State
 * @param si	Index
 */
static void imp_unlink(ImpState *s, int si) {
	int  si2 = si * 2;
	word bx = s->undo[si2 / 2];
	if (bx != IMP_NIL) {
		s->hash[bx / 2] = IMP_NIL;
		s->undo[si2 / 2] = IMP_NIL;
	}
}

/**
 * Find the longest matching string and insert lempel[di] into
 * the hash table.  Sets s->qlen and s->qoff.
 * Entry: di = current encode position (ring index, not DS offset).
 * The assembly proc adds offset(Lempel) to get DS-relative pointers for
 * string comparison.  In C we just index s->lempel[] directly.
 * The Ziv[] shadow array (s->lempel[IMP_LEMPEL .. IMP_LEMPEL+IMP_ZIV])
 * mirrors s->lempel[0..IMP_ZIV-1], so repe cmpsb can cross the ring
 * boundary without explicit wraparound -- comparisons starting near the
 * end of lempel[] continue into the shadow without going out of bounds.
 * AX encodes "remaining CX after repe cmpsb":
 * AX = ZIV-1 initially   (no match found yet)
 * AX = 0                 (ZIV-1 chars matched after hash char = ZIV total)
 * AX = 0xFFFF (-1 uint16) set by can1 for a full match (je from repe cmpsb)
 * Qlen = (uint16)(ZIV - AX) - 1  (wraps correctly for AX=0xFFFF -> Qlen=ZIV)
 * Qoff = -(( (di - best_pos/2) & (IMP_LEMPEL-1) ))
 * where best_pos is the BYTE OFFSET of the best candidate.
 * Match also links lempel[di] into the hash table (same as Link but
 * inlined at the end of can2, so it does NOT call imp_link separately
 * before the first Match call -- Fabrice calls Match before the main loop).
 *
 * @param s		State
 * @param di	Value
 */
static void imp_match(ImpState *s, int di) {
	int si_idx = imp_hash_key_idx(s, di);  // word index of key slot
	int di2 = di * 2;

	word ax = (word)(IMP_ZIV - 1);  // best "remaining CX" so far
	word bp = IMP_NIL;  // byte offset of best candidate

	// Walk the hash chain
	word bx = s->hash[si_idx];
	while (bx != IMP_NIL) {
		int cand = bx / 2;  // candidate position in lempel[]

		// Compare lempel[cand+1 .. cand+ZIV-1] against lempel[di+1 .. di+ZIV-1].
		// First character is guaranteed to match by the hash function.
		// The Ziv shadow allows comparisons to run past lempel[8191] safely.
		int max_cmp = IMP_ZIV - 1;
		int matched = 0;
		while (matched < max_cmp &&
			s->lempel[cand + 1 + matched] == s->lempel[di + 1 + matched])
			matched++;

		if (matched == max_cmp) {
			// Full ZIV-1 chars matched (= can1: full ZIV match with hash char)
			bp = bx;
			ax = (word)(-1);  // sentinel: 0xFFFF
			break;
		}

		// Remaining = max_cmp - matched.  Lower remaining = longer match.
		word cx = (word)(max_cmp - matched);
		if (cx < ax) {
			bp = bx;
			ax = cx;
		}

		bx = s->hash[bx / 2];  // follow collision chain
	}

	// Compute Qlen: (uint16)(ZIV - ax) - 1
	// Works for ax=0xFFFF: (uint16)(253 - 65535) = 254, -1 = 253 = ZIV.
	word qlen = (word)(IMP_ZIV - ax) - 1;
	s->qlen = qlen;

	// Compute Qoff (only meaningful when qlen >= 2):
	// Assembly: ax = di2 - bp; shr ax,1; and ax, LEMPEL-1; neg ax
	if (qlen >= 2) {
		word diff = (word)((word)di2 - bp);
		diff >>= 1;
		diff &= (IMP_LEMPEL - 1);
		s->qoff = -(int16_t)diff;
	}

	// Insert di into hash table (mirrors the inlined Link at end of can2).
	// bx here is the current root of the key slot (from the final iteration
	// of the chain walk, or the initial value if chain was empty).
	bx = s->hash[si_idx];  // re-read current root
	s->hash[di2 / 2] = bx;
	s->hash[si_idx] = (word)di2;
	s->undo[bx / 2] = (word)di2;
	s->undo[di2 / 2] = (word)(si_idx * 2);
}

/**
 * Initialise I/O pointers.  Mirrors RWinit proc.
 *
 * @param s		State
 */
static void imp_rwinit(ImpState *s) {
	s->rb_cnt = 1;  // triggers refill on first access
	s->wb_ptr = s->wb;
	s->wb_cnt = IMP_WBLEN;
}

/**
 * The main LZ77 encode loop.
 * Corresponds to the Fabrice proc in IMPLODE.ASM.
 * DI (ring pos) = current encode/input position in lempel[] ring.
 * SI (ring pos) = oldest position (evicted from dictionary as we advance).
 * Both wrap mod IMP_LEMPEL via "and si/di, LEMPEL-1".
 *
 * @param s		State
 */
static void imp_fabrice(ImpState *s) {
	int di, si;

	imp_rwinit(s);

	// Initialise UnDo to NIL
	for (int i = 0; i <= IMP_LEMPEL; i++)
		s->undo[i] = IMP_NIL;

	// Initialise Hash to NIL
	for (int i = 0; i < IMP_LEMPEL + 1 + IMP_HASH; i++)
		s->hash[i] = IMP_NIL;

	// Zero Lempel + Ziv area
	memset(s->lempel, 0, IMP_LEMPEL + IMP_ZIV - 1);

	// Initialise Huffman packet state
	s->huff_bit = 1;
	s->huff_man = 0;
	memset(s->packet, 0, IMP_PKTMAX);
	s->huff_ptr = s->packet + 2;

	// Initialise LZ state
	s->norms = 0;
	s->delta = 0;
	s->paraz = 0;

	// Read the first IMP_ZIV bytes into lempel[IMP_LEMPEL-ZIV .. IMP_LEMPEL-1].
	// Also mirror the first ZIV-1 of them into the Ziv shadow at
	// lempel[IMP_LEMPEL .. IMP_LEMPEL + ZIV - 2].
	//
	// Assembly:
	//   di = offset Ziv - ZIV  (= Lempel + LEMPEL - ZIV in DS-relative)
	//   cx = ZIV
	// Loop1: dec RBcnt; jnz @F; call implode_RBin; jz GotZIV; jmp Loop1
	// @@:    mov si,RBptr; inc RBptr; movsb; loop Loop1
	//
	// In flat C: write to lempel[IMP_LEMPEL - ZIV + i], and also to
	// lempel[IMP_LEMPEL + i] for i < ZIV-1 (the Ziv shadow).
	di = IMP_LEMPEL - IMP_ZIV;  // ring index: encode ptr starts here
	si = 0;  // ring index: oldest (tail) ptr
	s->zlen = 0;

	for (int i = 0; i < IMP_ZIV; i++) {
		int ch = imp_get_byte(s);
		if (ch < 0) break;
		s->lempel[IMP_LEMPEL - IMP_ZIV + i] = (byte)ch;
		if (i < IMP_ZIV - 1)
			s->lempel[IMP_LEMPEL + i] = (byte)ch;  // Ziv shadow
		s->zlen++;
	}

	// GotZIV: Zlen = min(ZIV, file_size).  Call initial Match.
	imp_match(s, di);

	// Main encode loop (Next label in assembly).
	while (s->zlen > 0) {

		// Clip Qlen to available lookahead
		if (s->qlen > s->zlen)
			s->qlen = s->zlen;

		int save_di = di;
		int save_si = si;

		// Decide encoding type.
		//
		// Assembly:
		//   cmp ax, 2; jb DoLit   => Qlen < 2: literal
		//   ja DoMov               => Qlen > 2: copy
		//   ; Qlen == 2:
		//   cmp Qoff, -100h; jg DoMov   => Qoff > -256: copy; else literal
		int qlen = (int)(int16_t)s->qlen;  // treat as signed for comparison
		int qoff = (int)s->qoff;

		if (qlen < 2 || (qlen == 2 && qoff <= -256)) {
			// DoLit: emit a literal byte.
			// Huff bit 1, then push the raw byte.
			// Qlen forced to 1 so we advance by exactly 1 character.
			s->qlen = 1;
			imp_huff1(s, 1);
			imp_push1(s, s->lempel[di]);

		} else {
			imp_huff1(s, 0);  // first Huff bit = 0 for all copies

			if (qlen <= 5 && qoff > -256) {
				// NearCopy: Qlen in [2..5], Qoff in [-255..-1].
				// Code: <00XY>, offset_byte.
				// XY encodes Qlen-2 as 2 bits: bit1=(enc>>1)&1, bit0=enc&1.
				// Offset byte = -(Qoff) = positive offset magnitude.
				//
				// Assembly:
				//   Huff1(0); Huff1(0)
				//   mov ax,Qlen; dec ax; dec ax; push ax
				//   and al,2; Huff1(al)   (= bit1 of enc)
				//   pop ax; and al,1; Huff1(al)  (= bit0 of enc)
				//   mov ax,Qoff; call push1  (low byte = -offset)
				imp_huff1(s, 0);
				int enc = qlen - 2;  // 0..3
				imp_huff1(s, (enc >> 1) & 1);  // bit1
				imp_huff1(s, enc & 1);  // bit0
				imp_push1(s, (byte)(-qoff));  // positive offset byte

			} else {
				// FarCopy: Qlen in [2..9] and Qoff in [-8191..-1], or Qlen > 5.
				// Code: <01>, then a 2-byte vector word, then optional length byte.
				//
				// Vector word format:
				//   Low  byte: Qoff & 0xFF  (offset low 8 bits)
				//   High byte: (Qoff >> 8) << 3  ORed with length field
				//     Length field (3 bits):
				//       0       => LongCopy (aux length byte follows)
				//       1..7    => FarCopy, length = field + 2  (range [3..9])
				//
				// Note: Qlen == 2 and Qoff <= -256 hits DoLit (above), so
				// minimum FarCopy length is 2 with Qoff <= -256... actually
				// the assembly checks: DoMov if Qlen>5 OR Qoff<=-256 when Qlen==2.
				// But "Far5" label handles Qlen>5 OR Qoff<=-255.
				// For Qlen==2 and Qoff<=-256 we already went to DoLit.
				// So here Qlen >= 2 and (Qlen > 5 OR Qoff <= -256 OR Qlen==2 case not reached).
				//
				// Assembly Far5:
				//   Huff1(1)
				//   mov ax,Qoff; push1(al)         [low byte of offset]
				//   mov al,ah; shl al,3             [high byte, shifted up 3]
				//   mov bx,Qlen; cmp bx,9; ja Far9
				//   dec bx; dec bx; or al,bl; push1(al)  [length in low 3 bits]
				// Far9: (LongCopy, Qlen > 9)
				//   push1(al)          [high byte with length=0]
				//   mov ax,bx; dec ax; push1(al)   [Qlen-1 as aux byte]
				imp_huff1(s, 1);
				imp_push1(s, (byte)(qoff & 0xFF));  // offset low byte
				byte hi = (byte)(((qoff >> 8) & 0xFF) << 3);  // hi byte, length field = 0

				if (qlen <= 9) {
					hi |= (byte)(qlen - 2);  // pack length into low 3 bits
					imp_push1(s, hi);
				} else {
					imp_push1(s, hi);  // length field = 0 => aux follows
					imp_push1(s, (byte)(qlen - 1));  // aux length byte
				}
			}
		}

		// Cont: bookkeeping after emitting the code.
		//
		// Assembly:
		//   sub Delta, Qlen; jge Zok
		// @@: inc ParaZ; add Delta,10h; cmp Delta,0; jl @B
		s->delta -= (int16_t)s->qlen;
		while (s->delta < 0) {
			s->paraz++;
			s->delta += 0x10;
		}

		// Norms tracking: emit a NORMALIZE code every ~40 KB (0xA000 bytes).
		// The NORMALIZE code tells explode to re-normalise its segment pointers;
		// in the flat C decompressor it is a no-op, but we must emit it to
		// keep the bitstream format compatible.
		//
		// Code: Huff(0), Huff(1), push(0x00), push(0xF0), push(0x01)
		s->norms = (word)(s->norms + s->qlen);
		if (s->norms >= 0xA000) {
			imp_huff1(s, 0);
			imp_huff1(s, 1);
			imp_push1(s, 0x00);
			imp_push1(s, 0xF0);
			imp_push1(s, 0x01);
			s->norms = 0;
		}

		// Restore DI and SI, then advance through the input by Qlen characters,
		// maintaining the hash dictionary.
		//
		// Assembly (Loop2):
		//   call Link     ; add lempel[di] to dict  (skipped on first iteration
		//   call UnLink   ;   via the "jmp short @F" before Loop2)
		//   ; read next character into lempel[si]
		//   if si < ZIV-1: also write lempel[LEMPEL+si] (Ziv shadow)
		//   inc si; inc di; and si,LEMPEL-1; and di,LEMPEL-1
		//   dec Qlen; jnz Loop2
		//
		// Note: on the FIRST iteration we skip Link (jump to UnLink directly)
		// because the current di was already linked by Match().
		di = save_di;
		si = save_si;

		int qlen_advance = (int)(int16_t)s->qlen;
		for (int k = 0; k < qlen_advance; k++) {
			if (k > 0) {
				// Link lempel[di] into dictionary (skipped on k=0 since
				// Match already inserted it).
				imp_link(s, di);
			}

			// UnLink the oldest entry (evict from tail of window)
			imp_unlink(s, si);

			// Read next character and store it at lempel[si]
			int ch = imp_get_byte(s);
			if (ch >= 0) {
				s->lempel[si] = (byte)ch;
				if (si < IMP_ZIV - 1)
					s->lempel[IMP_LEMPEL + si] = (byte)ch;  // Ziv shadow
			} else {
				// EOF: Zin label -- shrink Zlen
				s->zlen--;
			}

			si = (si + 1) & (IMP_LEMPEL - 1);
			di = (di + 1) & (IMP_LEMPEL - 1);
		}

		// Find next match for the new di position
		imp_match(s, di);
	}

	// Exit: emit the EXIT sentinel code, flush packet and write buffer.
	// Code: Huff(0), Huff(1), push(0x00), push(0xF0), push(0x00)
	imp_huff1(s, 0);
	imp_huff1(s, 1);
	imp_push1(s, 0x00);
	imp_push1(s, 0xF0);
	imp_push1(s, 0x00);

	imp_packet(s);  // WBend: flush final packet
	imp_wb_flush(s);  // flush write buffer
}

/**
 * LZ77 compressor.
 *
 * type and dsize are accepted for API compatibility but ignored; the
 * assembly original also ignored them (always binary mode, 8 KB window).
 * work_buff is ignored in the flat C port (state lives in heap via ImpState).
 */
word implode(
	word(*read_buff) (char *buffer, word *size),
	word(*write_buff)(char *buffer, word *size),
	char *work_buff,
	word *type,
	word *dsize) {
	(void)work_buff;
	(void)type;
	(void)dsize;

	// ImpState is large (~53 KB); allocate on heap, not stack.
	ImpState *s = (ImpState *)malloc(sizeof(ImpState));
	if (!s) return CMP_ABORT;
	memset(s, 0, sizeof * s);

	s->read_buff = read_buff;
	s->write_buff = write_buff;

	imp_fabrice(s);

	free(s);
	return CMP_NO_ERROR;
}

long crc32(unsigned char *buffer, word *size, long *old_crc) {
	// Build table at first call for polynomial 0xEDB88320 (reflected CRC-32)
	static uint32_t crc_table[256];
	static int table_init = 0;
	if (!table_init) {
		for (int i = 0; i < 256; i++) {
			uint32_t c = (uint32_t)i;
			for (int j = 0; j < 8; j++)
				c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
			crc_table[i] = c;
		}
		table_init = 1;
	}

	uint32_t crc = (uint32_t)(~(*old_crc));
	word len = *size;
	for (word i = 0; i < len; i++)
		crc = (crc >> 8) ^ crc_table[(crc ^ buffer[i]) & 0xFF];
	*old_crc = (long)(~crc);
	return *old_crc;
}

} // namespace MADSV2
} // namespace MADS
