#ifndef ENGINES_DREAMGEN_RUNTIME_H__
#define ENGINES_DREAMGEN_RUNTIME_H__

#include <assert.h>
#include "common/scummsys.h"
#include "common/array.h"
#include "common/hashmap.h"

namespace dreamgen {

//fixme: name clash
#undef random

struct LowPartOfRegister {
	enum { Mask = 0xff };
	enum { Shift = 0 };
};
struct HighPartOfRegister {
	enum { Mask = 0xff00 };
	enum { Shift = 8 };
};

struct Register {
	uint16 _value;
	inline Register(): _value() {}
	inline Register& operator=(uint16 v) { _value = v; return *this; }
	inline operator uint16&() { return _value; }
	inline void cbw() {
		if (_value & 0x80)
			_value |= 0xff00;
		else
			_value &= 0xff;
	}
};

template<typename P>
struct RegisterPart {
	typedef P 		PartTraits;
	Register		&_reg;
	uint8			_value;

	inline RegisterPart(Register &reg) : _reg(reg), _value(reg._value >> PartTraits::Shift) {}

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
	inline ~RegisterPart() {
		_reg._value = (_reg._value & ~PartTraits::Mask) | (_value << PartTraits::Shift);
	}
};

class WordRef {
	Common::Array<uint8>	&_data;
	unsigned				_index;
	uint16					_value;

public:

	inline WordRef(Common::Array<uint8> &data, unsigned index) : _data(data), _index(index) {
		assert(index + 1 < data.size());
		_value = _data[index] | (_data[index + 1] << 8);
	}
	inline WordRef& operator=(const WordRef &ref) {
		_value = ref._value;
		return *this;
	}
	inline WordRef& operator=(uint16 v) {
		_value = v;
		return *this;
	}
	inline operator uint16() const {
		return _value;
	}
	inline operator uint16&() {
		return _value;
	}
	inline ~WordRef() {
		_data[_index] = _value & 0xff;
		_data[_index + 1] = _value >> 8;
	}
};

struct Segment {
	Common::Array<uint8> data;
	
	inline void assign(const uint8 *b, const uint8 *e) {
		data.assign(b, e);
	}
	
	inline uint8 &byte(unsigned index) {
		assert(index < data.size());
		return data[index];
	}
	
	inline uint16 word(unsigned index) const {
		assert(index + 1 < data.size());
		return data[index] | (data[index + 1] << 8);
	}

	inline WordRef word(unsigned index) {
		return WordRef(data, index);
	}
};

class Context;

class SegmentRef {
	Context			*_context;
	uint16			_value;
	Segment			*_segment;

public:
	SegmentRef(Context *ctx, uint16 value = 0, Segment *segment = 0): _context(ctx), _value(value), _segment(segment) {
	}

	inline void reset(uint16 value);

	inline SegmentRef& operator=(const uint16 id) {
		reset(id);
		return *this;
	}

	inline uint8 &byte(unsigned index) {
		assert(_segment != 0);
		return _segment->byte(index);
	}

	inline uint16 word(unsigned index) const {
		assert(_segment != 0);
		return _segment->word(index);
	}
	
	inline operator uint16() const {
		return _value;
	}

	inline WordRef word(unsigned index) {
		assert(_segment != 0);
		return _segment->word(index);
	}

	inline void assign(const uint8 *b, const uint8 *e) {
		assert(_segment != 0);
		_segment->assign(b, e);
	}
};

struct Flags {
	bool _z, _c, _s, _o;
	inline Flags(): _z(true), _c(false), _s(false), _o(false) {}

	inline bool z() const { return _z; }
	inline bool c() const { return _c; }
	inline bool s() const { return _s; }
	//complex flags:
	inline bool g() const { return !_z && _s == _o; }
	inline bool ge() const { return _z || _s == _o; }
	inline bool l() const { return !_z && _s != _o; }
	inline bool le() const { return _z || _s != _o; }
	
	inline void update_sign(uint8 v) {
		bool new_s = v & 0x80;
		_o = new_s != _s;
		_s = new_s;
		_z = v == 0;
	}

	inline void update(uint16 v) {
		bool new_s = v & 0x8000;
		_o = new_s != _s;
		_s = new_s;
		_z = v == 0;
	}
};

class Context {
	typedef Common::HashMap<uint16, Segment> SegmentMap;
	SegmentMap _segments;
	
public:
	enum { kDefaultDataSegment = 0x1000 };
	
	Register ax, dx, bx, cx, si, di;
	RegisterPart<LowPartOfRegister> al;
	RegisterPart<HighPartOfRegister> ah;
	RegisterPart<LowPartOfRegister> bl;
	RegisterPart<HighPartOfRegister> bh;
	RegisterPart<LowPartOfRegister> cl;
	RegisterPart<HighPartOfRegister> ch;
	RegisterPart<LowPartOfRegister> dl;
	RegisterPart<HighPartOfRegister> dh;
	
	SegmentRef cs, ds, es;
	Flags flags;

	inline Context(): al(ax), ah(ax), bl(bx), bh(bx), cl(cx), ch(cx), dl(dx), dh(dx), cs(this), ds(this), es(this) {
		_segments[kDefaultDataSegment] = Segment();
		cs.reset(kDefaultDataSegment);
		ds.reset(kDefaultDataSegment);
		es.reset(kDefaultDataSegment);
	}
	
	SegmentRef getSegment(uint16 value) {
		SegmentMap::iterator i = _segments.find(value);
		assert(i != _segments.end());
		return SegmentRef(this, value, &i->_value);
	}
	
	SegmentRef allocateSegment(uint size) {
		unsigned id = kDefaultDataSegment + _segments.size();
		assert(!_segments.contains(id));
		Segment &seg = _segments[id];
		seg.data.resize(size);
		return SegmentRef(this, id, &seg);
	}
	
	inline void _cmp(uint8 a, uint8 b) {
		uint8 x = a;
		_sub(x, b);
	}
	inline void _cmp(uint16 a, uint16 b) {
		uint16 x = a;
		_sub(x, b);
	}
	inline void _test(uint8 a, uint8 b) {
		uint8 x = a;
		_and(x, b);
	}
	inline void _test(uint16 a, uint16 b) {
		uint16 x = a;
		_and(x, b);
	}
	
	inline void _add(uint8 &dst, uint8 src) {
		flags._c = dst + src < dst;
		dst += src;
		flags.update(dst);
	}
	inline void _add(uint16 &dst, uint16 src) {
		flags._c = dst + src < dst;
		dst += src;
		flags.update(dst);
	}
	inline void _sub(uint8 &dst, uint8 src) {
		flags._c = dst < src;
		dst -= src;
		flags.update(dst);
	}
	inline void _sub(uint16 &dst, uint16 src) {
		flags._c = dst < src;
		dst -= src;
		flags.update(dst);
	}

	inline void _and(uint8 &dst, uint8 src) {
		dst &= src;
		flags._c = false;
		flags.update(dst);
	}
	inline void _and(uint16 &dst, uint16 src) {
		dst &= src;
		flags._c = false;
		flags.update(dst);
	}
	inline void _or(uint8 &dst, uint8 src) {
		dst |= src;
		flags._c = false;
		flags.update(dst);
	}
	inline void _or(uint16 &dst, uint16 src) {
		dst |= src;
		flags._c = false;
		flags.update(dst);
	}

	inline void _xor(uint8 &dst, uint8 src) {
		dst ^= src;
		flags._c = false;
		flags.update(dst);
	}
	inline void _xor(uint16 &dst, uint16 src) {
		dst ^= src;
		flags._c = false;
		flags.update(dst);
	}

	inline void _shr(uint8 &dst, uint8 src) {}
	inline void _shr(uint16 &dst, uint8 src) {}
	inline void _shl(uint8 &dst, uint8 src) {}
	inline void _shl(uint16 &dst, uint8 src) {}
	inline void _mul(uint8 src) {
		unsigned r = unsigned(al) * src;
		ax = (uint16)r;
		flags._c = r >= 0x10000;
		flags._z = r == 0;
		bool s = r & 0x8000;
		flags._o = s != flags._s;
		flags._s = s;
	}
	inline void _mul(uint16 src) {
		unsigned r = unsigned(ax) * src; //assuming here that we have at least 32 bits
		dx = (r >> 16) & 0xffff;
		ax = r & 0xffff;
		flags._c = false;//fixme
		flags._z = r == 0;
		bool s = r & 0x80000000;
		flags._o = s != flags._s;
		flags._s = s;
	}
	inline void _neg(uint8 &src) {
		src = ~src;
		flags._c = false;
		flags.update(src);
	}
	inline void _neg(uint16 &src) {
		src = ~src;
		flags._c = false;
		flags.update(src);
	}

	inline void _movsb() {
		es.byte(di++) = ds.byte(si++);
	}
	inline void _movsw() {
		es.word(di) = ds.word(si);
		di += 2;
		si += 2;
	}
	inline void _lodsb() {
		al = ds.byte(si++);
	}
	inline void _lodsw() {
		ax = ds.word(si);
		si += 2;
	}
	inline void _stosb() {
		es.byte(di++) = al;
	}
	inline void _stosw() {
		es.word(di) = al;
		di += 2;
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
		uint16 v = stack.back();
		stack.pop_back();
		return v;
	}
};

inline void SegmentRef::reset(uint16 value) {
	*this = _context->getSegment(value);
}

}

#endif

