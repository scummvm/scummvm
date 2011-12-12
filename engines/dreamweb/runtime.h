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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef DREAMGEN_RUNTIME_H
#define DREAMGEN_RUNTIME_H

#include <assert.h>
#include "common/scummsys.h"
#include "common/array.h"
#include "common/debug.h"

#include "dreamweb/segment.h"

namespace DreamGen {

struct Register {
	union {
		uint16 _value;
		uint8 _part[2];
	};
	inline Register(): _value() {}
	inline Register& operator=(uint16 v) { _value = v; return *this; }
	inline operator uint16&() { return _value; }
};

template<int kIndex> //from low to high
struct RegisterPart {
	uint8			&_value;

	explicit inline RegisterPart(Register &reg) : _value(reg._part[kIndex]) {}

	inline operator uint8&() {
		return _value;
	}

	inline RegisterPart& operator=(const RegisterPart& o) {
		_value = o._value;
		return *this;
	}

	inline RegisterPart& operator=(uint8 v) {
		_value = v;
		return *this;
	}
};

#ifdef SCUMM_LITTLE_ENDIAN
	typedef RegisterPart<0> LowPartOfRegister;
	typedef RegisterPart<1> HighPartOfRegister;
#else
	typedef RegisterPart<1> LowPartOfRegister;
	typedef RegisterPart<0> HighPartOfRegister;
#endif

struct Flags {
	bool _z, _c, _s, _o;
	inline Flags(): _z(true), _c(false), _s(false), _o(false) {}

	inline bool z() const	{ return _z; }
	inline bool c() const	{ return _c; }
	inline bool s() const	{ return _s; }

	inline bool l() const	{ return _o != _s; }
	inline bool le() const	{ return _o != _s|| _z; }

	inline void update_zs(uint8 v) {
		_s = v & 0x80;
		_z = v == 0;
	}

	inline void update_zs(uint16 v) {
		_s = v & 0x8000;
		_z = v == 0;
	}

	inline void update_o(uint8 v, uint8 a, uint8 b) {
		uint8 s1 = a & 0x80, s2 = b & 0x80;
		_o = (s1 == s2) && (v & 0x80) != s1;
	}

	inline void update_o(uint16 v, uint16 a, uint16 b) {
		uint16 s1 = a & 0x8000, s2 = b & 0x8000;
		_o = (s1 == s2) && (v & 0x8000) != s1;
	}
};

class Context {
public:
	Register ax, dx, bx, cx, si, di;
	LowPartOfRegister	al;
	HighPartOfRegister	ah;
	LowPartOfRegister	bl;
	HighPartOfRegister	bh;
	LowPartOfRegister	cl;
	HighPartOfRegister	ch;
	LowPartOfRegister	dl;
	HighPartOfRegister	dh;

	SegmentRef cs;
	MutableSegmentRef ds;
	MutableSegmentRef es;
	Flags flags;

	Context(SegmentManager *segMan): al(ax), ah(ax), bl(bx), bh(bx), cl(cx), ch(cx), dl(dx), dh(dx),
		cs(segMan->data),
		ds(segMan, segMan->data),
		es(segMan, segMan->data) {

	}

	inline void _cmp(uint8 a, uint8 b) {
		_sub(a, b);
	}

	inline void _cmp(uint16 a, uint16 b) {
		_sub(a, b);
	}

	inline void _test(uint8 a, uint8 b) {
		_and(a, b);
	}

	inline void _test(uint16 a, uint16 b) {
		_and(a, b);
	}

	inline void _add(uint8 &dst, uint8 src) {
		unsigned r = (unsigned)dst + src;
		flags.update_o((uint8)r, dst, src);
		flags._c = r >= 0x100;
		dst = r;
		flags.update_zs(dst);
	}

	inline void _add(uint16 &dst, uint16 src) {
		unsigned r = (unsigned)dst + src;
		flags.update_o((uint16)r, dst, src);
		flags._c = r >= 0x10000;
		dst = r;
		flags.update_zs(dst);
	}

	inline void _sub(uint8 &dst, uint8 src) {
		flags.update_o(uint8(dst - src), dst, (uint8)-src);
		flags._c = dst < src;
		dst -= src;
		flags.update_zs(dst);
	}

	inline void _sub(uint16 &dst, uint16 src) {
		flags.update_o(uint16(dst - src), dst, (uint16)-src);
		flags._c = dst < src;
		dst -= src;
		flags.update_zs(dst);
	}

	inline void _inc(uint8 &dst) {
		flags.update_o((uint8)(dst + 1), dst, 1);
		++dst;
		flags.update_zs(dst);
	}

	inline void _inc(uint16 &dst) {
		flags.update_o((uint16)(dst + 1), dst, 1);
		++dst;
		flags.update_zs(dst);
	}

	inline void _dec(uint8 &dst) {
		flags.update_o(uint8(dst - 1), dst, 1);
		--dst;
		flags.update_zs(dst);
	}

	inline void _dec(uint16 &dst) {
		flags.update_o(uint16(dst - 1), dst, 1);
		--dst;
		flags.update_zs(dst);
	}

	inline void _and(uint8 &dst, uint8 src) {
		dst &= src;
		flags.update_zs(dst);
		flags._c = flags._o = false;
	}

	inline void _and(uint16 &dst, uint16 src) {
		dst &= src;
		flags.update_zs(dst);
		flags._c = flags._o = false;
	}

	inline void _or(uint8 &dst, uint8 src) {
		dst |= src;
		flags.update_zs(dst);
		flags._c = flags._o = false;
	}

	inline void _or(uint16 &dst, uint16 src) {
		dst |= src;
		flags.update_zs(dst);
		flags._c = flags._o = false;
	}

	inline void _xor(uint8 &dst, uint8 src) {
		dst ^= src;
		flags.update_zs(dst);
		flags._c = flags._o = false;
	}

	inline void _xor(uint16 &dst, uint16 src) {
		dst ^= src;
		flags.update_zs(dst);
		flags._c = flags._o = false;
	}

	inline void _shr(uint8 &dst, uint8 src) {
		src &= 0x1f;
		if (src > 0) {
			dst >>= (src - 1);
			flags._c = dst & 1;
			dst >>= 1;
			flags.update_zs(dst);
		}
		if (src == 1)
			flags._o = dst & 0x80;
	}

	inline void _shr(uint16 &dst, uint8 src) {
		src &= 0x1f;
		if (src > 0) {
			dst >>= (src - 1);
			flags._c = dst & 1;
			dst >>= 1;
			flags.update_zs(dst);
		}
		if (src == 1)
			flags._o = dst & 0x8000;
	}

	inline void _shl(uint8 &dst, uint8 src) {
		src &= 0x1f;
		if (src > 0) {
			dst <<= (src - 1);
			flags._c = dst & 0x80;
			dst <<= 1;
			flags.update_zs(dst);
		}
		if (src == 1)
			flags._o = ((dst & 0x80) != 0) == flags._c;
	}
	inline void _shl(uint16 &dst, uint8 src) {
		src &= 0x1f;
		if (src > 0) {
			dst <<= (src - 1);
			flags._c = dst & 0x8000;
			dst <<= 1;
			flags.update_zs(dst);
		}
		if (src == 1)
			flags._o = ((dst & 0x8000) != 0) == flags._c;
	}

	inline void _mul(uint8 src) {
		unsigned r = unsigned(al) * src;
		ax = (uint16)r;
		flags._c = r >= 0x10000;
		flags._z = r == 0;
		flags._s = r & 0x8000;
		flags._o = ah != 0;
	}

	inline void _mul(uint16 src) {
		unsigned r = unsigned(ax) * src; //assuming here that we have at least 32 bits
		dx = (r >> 16) & 0xffff;
		ax = r & 0xffff;
		flags._c = false;
		flags._z = r == 0;
		flags._s = r & 0x80000000;
		flags._o = dx != 0;
	}

	inline void _neg(uint8 &src) {
		uint8 r = 0;
		_sub(r, src);
		src = r;
	}

	inline void _neg(uint16 &src) {
		uint16 r = 0;
		_sub(r, src);
		src = r;
	}

	inline void _lodsb() {
		al = ds.byte(si++);
	}

	inline void _lodsw() {
		ax = ds.word(si);
		si += 2;
	}

	inline void _movsb() {
		es.byte(di++) = ds.byte(si++);
	}

	inline void _movsb(uint size, bool clear_cx = false) {
		assert(size != 0xffff);
		//fixme: add overlap and segment boundary check and rewrite
		while (size--)
			_movsb();
		if (clear_cx)
			cx = 0;
	}

	inline void _movsw() {
		_movsb();
		_movsb();
	}

	inline void _movsw(uint size, bool clear_cx = false) {
		assert(size != 0xffff);
		_movsb(size * 2, clear_cx);
	}

	inline void _stosb() {
		es.byte(di++) = al;
	}

	inline void _stosb(uint size, bool clear_cx = false) {
		assert(size != 0xffff);
		uint8 *dst = es.ptr(di, size);
		memset(dst, al, size);
		di += size;
		if (clear_cx)
			cx = 0;
	}

	inline void _stosw() {
		es.byte(di++) = al;
		es.byte(di++) = ah;
	}

	inline void _stosw(uint size, bool clear_cx = false) {
		assert(size != 0xffff);
		uint8 *dst = es.ptr(di, size * 2);
		di += 2 * size;
		while (size--) {
			*dst++ = al;
			*dst++ = ah;
		}
		if (clear_cx)
			cx = 0;
	}

	inline void _xchg(uint16 &a, uint16 &b) {
		uint16 x = a;
		a = b;
		b = x;
	}

	inline void _xchg(uint8 &a, uint8 &b) {
		uint8 t = a;
		a = b;
		b = t;
	}

	Common::Array<uint16> stack;
	inline void push(uint16 v) {
		stack.push_back(v);
	}

	inline uint16 pop() {
		assert(!stack.empty());
		uint16 v = stack.back();
		stack.pop_back();
		return v;
	}
};

class StackChecker {
	const Context	&_context;
	const uint		_stackDepth;

public:
	StackChecker(const Context &context): _context(context), _stackDepth(context.stack.size()) {}
	~StackChecker() { assert(_context.stack.size() == _stackDepth); }
};

#ifndef NDEBUG
#	define STACK_CHECK  StackChecker checker(*this)
#else
#	define STACK_CHECK  do {} while (0)
#endif

} // End of namespace DreamGen

#endif
