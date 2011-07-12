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

/*
 * This code is based on original Soltys source code
 * Copyright (c) 1994-1995 Janus B. Wisniewski and L.K. Avalon
 */

#ifndef __CGE_GENERAL__
#define __CGE_GENERAL__

#include "common/system.h"
#include "common/file.h"
#include "common/random.h"
#include "common/textconsole.h"
#include "common/str.h"
#include "cge/jbw.h"

namespace CGE {

#define     SEED        0xA5

enum    MEM_TYPE    { BAD_MEM, EMS_MEM, NEAR_MEM, FAR_MEM };
enum    IOMODE      { REA, WRI, UPD };

struct Dac {
	uint8 _r;
	uint8 _g;
	uint8 _b;
};

typedef uint16  CRYPT(void *buf, uint16 siz, uint16 seed);

class Couple {
protected:
	signed char _a;
	signed char _b;
public:
	Couple() { }
	Couple(const signed char a, const signed char b) : _a(a), _b(b) { }
	Couple operator + (Couple c) {
		return Couple(_a + c._a, _b + c._b);
	}

	void operator += (Couple c) {
		_a += c._a;
		_b += c._b;
	}

	Couple operator - (Couple c) {
		return Couple(_a - c._a, _b - c._b);
	}

	void operator -= (Couple c) {
		_a -= c._a;
		_b -= c._b;
	}

	bool operator == (Couple c) {
		return ((_a - c._a) | (_b - c._b)) == 0;
	}

	bool operator != (Couple c) {
		return !(operator == (c));
	}

	void split(signed char &a, signed char &b) {
		a = _a;
		b = _b;
	}
};


class Engine_ {
protected:
	static void (* oldTimer)(...);
	static void newTimer(...);
public:
	Engine_(uint16 tdiv);
	~Engine_();
};


class Ems;


class Emm {
	friend class Ems;
	bool test();

	long _top;
	long _lim;
	Ems *_list;
	int _han;
	static void *_frame;
public:
	Emm(long size = 0);
	~Emm();
	Ems *alloc(uint16 siz);
	void release();
};


class Ems {
	friend class Emm;
	Emm *_emm;
	long _ptr;
	uint16 _size;
	Ems *_next;
public:
	Ems();
	void *operator & () const;
	uint16 size();
};


template <class T>
void swap(T &A, T &B) {
	T a = A;
	A = B;
	B = a;
}


#ifdef __cplusplus
template <class T>
T max(T A, T B) {
	return (A > B) ? A : B;
}

template <class T>
T min(T A, T B) {
	return (A < B) ? A : B;
}
#endif


class XFile {
public:
	IOMODE _mode;
	uint16 _error;

	XFile() : _mode(REA), _error(0) { }
	XFile(IOMODE mode) : _mode(mode), _error(0) { }
	virtual ~XFile() { }
	virtual uint16 read(void *buf, uint16 len) = 0;
	virtual uint16 write(void *buf, uint16 len) = 0;
	virtual long mark() = 0;
	virtual long size() = 0;
	virtual long seek(long pos) = 0;
};


template <class T>
inline uint16 XRead(XFile *xf, T *t) {
	return xf->read((uint8 *) t, sizeof(*t));
}


class IoHand : public XFile {
protected:
	Common::File *_file;
	uint16 _seed;
	CRYPT *_crypt;
public:
	IoHand(const char *name, IOMODE mode = REA, CRYPT crypt = NULL);
	IoHand(IOMODE mode = REA, CRYPT *crpt = NULL);
	virtual ~IoHand();
	static bool exist(const char *name);
	uint16 read(void *buf, uint16 len);
	uint16 write(void *buf, uint16 len);
	long mark();
	long size();
	long seek(long pos);
	//timeb  Time ();
// void SetTime (timeb  t);
};


CRYPT     XCrypt;
CRYPT     RCrypt;
MEM_TYPE  memType(void *mem);
uint16    atow(const char *a);
uint16    xtow(const char *x);
char     *wtom(uint16 val, char *str, int radix, int len);
char     *dwtom(uint32 val, char *str, int radix, int len);
int       takeEnum(const char **tab, const char *txt);
uint16    chkSum(void *m, uint16 n);
long      timer();
char     *mergeExt(char *buf, const char *nam, const char *ext);
char     *forceExt(char *buf, const char *nam, const char *ext);
int       driveCD(unsigned drv);
bool      isVga();


// MISSING FUNCTIONS
void _fqsort(void *base, uint16 nelem, uint16 width, int (*fcmp)(const void *, const void *));
const char *progName(const char *ext = NULL);
char *mergeExt(char *buf, const char *nam, const char *ext);
char *forceExt(char *buf, const char *nam, const char *ext);
unsigned fastRand();
unsigned fastRand(unsigned s);
uint16 rCrypt(void *buf, uint16 siz, uint16 seed);
uint16 atow(const char *a);
uint16 xtow(const char *x);
char *wtom(uint16 val, char *str, int radix, int len);
char *dwtom(uint32 val, char * str, int radix, int len);
int takeEnum(const char **tab, const char *txt);
long timer();
int new_random(int range);
} // End of namespace CGE

#endif
