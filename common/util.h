/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2004 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef COMMON_UTIL_H
#define COMMON_UTIL_H

#include "common/scummsys.h"
#include "common/system.h"

template<typename T> inline T ABS (T x)			{ return (x>=0) ? x : -x; }
template<typename T> inline T MIN (T a, T b)	{ return (a<b) ? a : b; }
template<typename T> inline T MAX (T a, T b)	{ return (a>b) ? a : b; }

/**
 * Template method which swaps the vaulues of its two parameters.
 */
template<typename T> inline void SWAP(T &a, T &b) { T tmp = a; a = b; b = tmp; }

#define ARRAYSIZE(x) ((int)(sizeof(x) / sizeof(x[0])))

namespace Common {

class String;

/**
 * Print a hexdump of the data passed in. The number of bytes per line is
 * customizable.
 * @param data	the data to be dumped
 * @param len	the lenght of that data
 * @param bytesPerLine	number of bytes to print per line (default: 16)
 */
extern void hexdump(const byte * data, int len, int bytesPerLine = 16);

/**
 * Simple random number generator. Although it is definitely not suitable for
 * cryptographic purposes, it serves our purposes just fine.
 */
class RandomSource {
private:
	uint32 _randSeed;

public:
	RandomSource();
	void setSeed(uint32 seed);

	/**
	 * Generates a random unsigned integer in the interval [0, max].
	 * @param max	the upper bound
	 * @return	a random number in the interval [0, max].
	 */
	uint getRandomNumber(uint max);
	/**
	 * Generates a random unsigned integer in the interval [min, max].
	 * @param min	the lower bound
	 * @param max	the upper bound
	 * @return	a random number in the interval [min, max].
	 */
	uint getRandomNumberRng(uint min, uint max);
};

/**
 * Auxillary class to (un)lock a mutex on the stack.
 */
class StackLock {
	OSystem::MutexRef _mutex;
	OSystem *_syst;
	void lock();
	void unlock();
public:
	StackLock(OSystem::MutexRef mutex, OSystem *syst = 0);
	~StackLock();
};

/**
 * List of language ids.
 * @note The order and mappings of the values 0..8 are *required* to stay the
 * way they are now, as scripts in COMI rely on them. So don't touch them.
 * I am working on removing this restriction.
 */
enum Language {
	UNK_LANG = -1,	// Use default language (i.e. none specified)
	EN_USA = 0,
	DE_DEU = 1,
	FR_FRA = 2,
	IT_ITA = 3,
	PT_BRA = 4,
	ES_ESP = 5,
	JA_JPN = 6,
	ZH_TWN = 7,
	KO_KOR = 8,
	SE_SWE = 9,
	EN_GRB = 10,
	HB_HEB = 20,
	RU_RUS = 21,
	CZ_CZE = 22
};

struct LanguageDescription {
	const char *code;
	const char *description;
	Common::Language id;
};

extern const LanguageDescription g_languages[];


/** Convert a string containing a language name into a Language enum value. */
extern Language parseLanguage(const String &str);
extern const char *getLanguageCode(Language id);
extern const char *getLanguageDescription(Language id);

/**
 * List of game platforms. Specifying a platform for a target can be used to
 * give the game engines a hint for which platform the game data file are.
 * This may be optional or required, depending on the game engine and the
 * game in question.
 */
enum Platform {
	kPlatformUnknown = -1,
	kPlatformPC = 0,
	kPlatformAmiga = 1,
	kPlatformAtariST = 2,
	kPlatformMacintosh = 3,
	kPlatformFMTowns = 4
/*
	kPlatformNES,
	kPlatformSEGA,
	kPlatformPCEngine
*/
};

struct PlatformDescription {
	const char *code;
	const char *code2;
	const char *description;
	Common::Platform id;
};

extern const PlatformDescription g_platforms[];


/** Convert a string containing a platform name into a Platform enum value. */
extern Platform parsePlatform(const String &str);
extern const char *getPlatformCode(Platform id);
extern const char *getPlatformDescription(Platform id);

}	// End of namespace Common



#if defined(__GNUC__)
void CDECL error(const char *s, ...) NORETURN;
#else
void CDECL NORETURN error(const char *s, ...);
#endif

void CDECL warning(const char *s, ...);

void CDECL debug(int level, const char *s, ...);
void CDECL debug(const char *s, ...);
void checkHeap();


#endif
