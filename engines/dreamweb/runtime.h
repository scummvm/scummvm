#ifndef ENGINES_DREAMGEN_RUNTIME_H__
#define ENGINES_DREAMGEN_RUNTIME_H__

#include <stdint.h>
#include <assert.h>
#include <vector>
#include "common/scummsys.h"

//fixme: name clash
#undef random

enum { kLowPartOfRegister = 0, kHighPartOfRegister = 1 };

struct Register {
	union {
		uint16 _value;
		uint8 _part[2];
	};
	inline Register(): _value() {}
	inline Register& operator=(uint16_t v) { _value = v; return *this; }
	inline operator uint16&() { return _value; }
	inline void cbw() {
		if (_value & 0x80)
			_value |= 0xff00;
		else
			_value &= 0xff;
	}
};

template<unsigned PART>
struct RegisterPart {
	uint8_t &_part;

	inline RegisterPart(Register &reg) : _part(reg._part[PART]) {}

	inline operator uint8&() {
		return _part;
	}
	inline RegisterPart& operator=(const RegisterPart& o) {
		_part = o._part;
		return *this;
	}
	inline RegisterPart& operator=(uint8_t v) {
		_part = v;
		return *this;
	}
};

struct WordRef {
	std::vector<uint8> &data;
	unsigned index;
	bool changed;
	uint16_t value;

	inline WordRef(std::vector<uint8> &data, unsigned index) : data(data), index(index), changed(false) {
		assert(index + 1 < data.size());
		value = data[index] | (data[index + 1] << 8);
	}
	inline WordRef& operator=(const WordRef &ref) {
		changed = true;
		value = ref.value;
		return *this;
	}
	inline WordRef& operator=(uint16_t v) {
		changed = true;
		value = v;
		return *this;
	}
	inline operator uint16_t() const {
		return value;
	}
	inline operator uint16_t&() {
		return value;
	}
	inline ~WordRef() {
		if (changed) {
			data[index] = value & 0xff;
			data[index + 1] = value >> 8;
		}
	}
};

struct Segment {
	std::vector<uint8> data;
	inline uint8_t &byte(unsigned index) {
		assert(index < data.size());
		return data[index];
	}
	inline uint16_t word(unsigned index) const {
		assert(index + 1 < data.size());
		return data[index] | (data[index + 1] << 8);
	}

	inline WordRef word(unsigned index) {
		return WordRef(data, index);
	}
};


struct SegmentRef {
	public:
	uint16_t value;
	std::vector<uint8> *data;
	inline SegmentRef& operator=(const SegmentRef &o) {
		data = o.data;
		return *this;
	}
	inline SegmentRef& operator=(const uint16_t id) {
		return *this;
	}
	inline uint8_t &byte(unsigned index) {
		assert(index < data->size());
		return (*data)[index];
	}
	inline uint16_t word(unsigned index) const {
		assert(index + 1 < data->size());
		return (*data)[index] | ((*data)[index + 1] << 8);
	}
	
	inline operator uint16_t() const {
		return value;
	}

	inline WordRef word(unsigned index) {
		return WordRef(*data, index);
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
		bool s = v & 0x80;
		_o = s != _s;
		_s = s;
		_z = v == 0;
	}

	inline void update(uint16 v) {
		bool s = v & 0x8000;
		_o = s != _s;
		_s = s;
		_z = v == 0;
	}
};

template<typename Data>
struct RegisterContext {
	Register ax, dx, bx, cx, si, di;
	RegisterPart<kLowPartOfRegister> al;
	RegisterPart<kHighPartOfRegister> ah;
	RegisterPart<kLowPartOfRegister> bl;
	RegisterPart<kHighPartOfRegister> bh;
	RegisterPart<kLowPartOfRegister> cl;
	RegisterPart<kHighPartOfRegister> ch;
	RegisterPart<kLowPartOfRegister> dl;
	RegisterPart<kHighPartOfRegister> dh;
	
	SegmentRef cs, ds, es;
	Flags flags;

	inline RegisterContext(): al(ax), ah(ax), bl(bx), bh(bx), cl(cx), ch(cx), dl(dx), dh(dx) {}

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

	std::vector<uint16> stack;
	inline void push(uint16 v) {
		stack.push_back(v);
	}
	inline uint16 pop() {
		uint16 v = stack.back();
		stack.pop_back();
		return v;
	}
	
	Data data;
};

#endif

