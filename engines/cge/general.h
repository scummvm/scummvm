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

#ifndef __GENERAL__
#define __GENERAL__

#include "common/system.h"
#include "common/file.h"
#include "common/random.h"
#include "common/textconsole.h"
#include "common/str.h"
#include "cge/jbw.h"
#include "cge/boot.h"

namespace CGE {

#define     SEED        0xA5

#define     SCR_WID_    320
#define     SCR_HIG_    200
#define     SCR_WID     ((uint16)SCR_WID_)
#define     SCR_HIG     ((uint16)SCR_HIG_)
#define     SCR_SEG     0xA000
#define     SCR_ADR     ((uint8 *) MK_FP(SCR_SEG, 0))



//enum  CPU     { _8086, _80186, _80286, _80386, _80486 };
enum    MEM_TYPE    { BAD_MEM, EMS_MEM, NEAR_MEM, FAR_MEM };
enum    ALLOC_MODE  { FIRST_FIT, BEST_FIT, LAST_FIT };
enum    IOMODE      { REA, WRI, UPD };

typedef struct  {
	uint8 R, G, B;
} DAC;

typedef uint16  CRYPT(void *buf, uint16 siz, uint16 seed);

class COUPLE {
protected:
	signed char A;
	signed char B;
public:
	COUPLE(void) { }
	COUPLE(const signed char a, const signed char b) : A(a), B(b) { }
	COUPLE operator + (COUPLE c) {
		return COUPLE(A + c.A, B + c.B);
	}

	void operator += (COUPLE c) {
		A += c.A;
		B += c.B;
	}

	COUPLE operator - (COUPLE c) {
		return COUPLE(A - c.A, B - c.B);
	}

	void operator -= (COUPLE c) {
		A -= c.A;
		B -= c.B;
	}

	bool operator == (COUPLE c) {
		return ((A - c.A) | (B - c.B)) == 0;
	}

	bool operator != (COUPLE c) {
		return !(operator == (c));
	}

	void Split(signed char &a, signed char &b) {
		a = A;
		b = B;
	}
};


class ENGINE {
protected:
	static void (* OldTimer)(...);
	static void NewTimer(...);
public:
	ENGINE(uint16 tdiv);
	~ENGINE(void);
};


class EMS;


class EMM {
	friend class EMS;
	bool Test(void);
	long Top, Lim;
	EMS *List;
	int Han;
	static void *Frame;
public:
	EMM(long size = 0);
	~EMM(void);
	EMS *Alloc(uint16 siz);
	void Release(void);
};


class EMS {
	friend class EMM;
	EMM *Emm;
	long Ptr;
	uint16 Siz;
	EMS *Nxt;
public:
	EMS(void);
	void *operator & () const;
	uint16 Size(void);
};


template <class T>
void Swap(T &A, T &B) {
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


class XFILE {
public:
	IOMODE Mode;
	uint16 Error;
	XFILE(void) : Mode(REA), Error(0) { }
	XFILE(IOMODE mode) : Mode(mode), Error(0) { }
	virtual uint16 read(void *buf, uint16 len) = 0;
	virtual uint16 write(void *buf, uint16 len) = 0;
	virtual long mark(void) = 0;
	virtual long size(void) = 0;
	virtual long seek(long pos) = 0;
	virtual ~XFILE() { }
};


template <class T>
inline uint16 XRead(XFILE *xf, T *t) {
	return xf->read((uint8 *) t, sizeof(*t));
}


class IoHand : public XFILE {
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
	//timeb  Time (void);
// void SetTime (timeb  t);
};


CRYPT     XCrypt;
CRYPT     RCrypt;
MEM_TYPE  MemType(void *mem);
uint16    atow(const char *a);
uint16    xtow(const char *x);
char     *wtom(uint16 val, char *str, int radix, int len);
char     *dwtom(uint32 val, char *str, int radix, int len);
int       TakeEnum(const char **tab, const char *txt);
uint16    ChkSum(void *m, uint16 n);
long      Timer(void);
char     *MergeExt(char *buf, const char *nam, const char *ext);
char     *ForceExt(char *buf, const char *nam, const char *ext);
int       DriveCD(unsigned drv);
bool      IsVga(void);


// MISSING FUNCTIONS
EC void     _fqsort(void *base, uint16 nelem, uint16 width, int (*fcmp)(const void *, const void *));
const char *ProgName(const char *ext = NULL);
char *MergeExt(char *buf, const char *nam, const char *ext);
char *ForceExt(char *buf, const char *nam, const char *ext);
unsigned FastRand(void);
unsigned FastRand(unsigned s);
uint16 RCrypt(void *buf, uint16 siz, uint16 seed);
uint16 atow(const char *a);
uint16 xtow(const char *x);
char *wtom(uint16 val, char *str, int radix, int len);
char *dwtom(uint32 val, char * str, int radix, int len);
int TakeEnum(const char **tab, const char *txt);
Boot *ReadBoot(int drive);
long Timer(void);
int new_random(int range);
} // End of namespace CGE

#endif
