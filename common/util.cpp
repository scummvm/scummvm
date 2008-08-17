/* Residual - Virtual machine to run LucasArts' 3D adventure games
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 */

#include "common/util.h"
#include "engine/backend/platform/driver.h"

#ifdef _WIN32_WCE
// This is required for the debugger attachment
extern bool isSmartphone(void);
#endif

#ifdef __PLAYSTATION2__
	// for those replaced fopen/fread/etc functions
	typedef unsigned long	uint64;
	typedef signed long	int64;
	#include "backends/platform/ps2/fileio.h"

	#define fprintf				ps2_fprintf
	#define fflush(a)			ps2_fflush(a)
#endif

#ifdef __DS__
	#include "backends/fs/ds/ds-fs.h"

	#undef stderr
	#undef stdout
	#undef stdin

	#define stdout ((DS::fileHandle*) -1)
	#define stderr ((DS::fileHandle*) -2)
	#define stdin ((DS::fileHandle*) -3)

	void	std_fprintf(FILE* handle, const char* fmt, ...);
	void	std_fflush(FILE* handle);

	#define fprintf(file, fmt, ...)				{ char str[128]; sprintf(str, fmt, ##__VA_ARGS__); DS::std_fwrite(str, strlen(str), 1, file); }
	#define fflush(file)						DS::std_fflush(file)
#endif


namespace Common {

//
// Print hexdump of the data passed in
//
void hexdump(const byte * data, int len, int bytesPerLine) {
	assert(1 <= bytesPerLine && bytesPerLine <= 32);
	int i;
	byte c;
	int offset = 0;
	while (len >= bytesPerLine) {
		printf("%06x: ", offset);
		for (i = 0; i < bytesPerLine; i++) {
			printf("%02x ", data[i]);
			if (i % 4 == 3)
				printf(" ");
		}
		printf(" |");
		for (i = 0; i < bytesPerLine; i++) {
			c = data[i];
			if (c < 32 || c >= 127)
				c = '.';
			printf("%c", c);
		}
		printf("|\n");
		data += bytesPerLine;
		len -= bytesPerLine;
		offset += bytesPerLine;
	}

	if (len <= 0)
		return;

	printf("%06x: ", offset);
	for (i = 0; i < bytesPerLine; i++) {
		if (i < len)
			printf("%02x ", data[i]);
		else
			printf("   ");
		if (i % 4 == 3)
			printf(" ");
	}
	printf(" |");
	for (i = 0; i < len; i++) {
		c = data[i];
		if (c < 32 || c >= 127)
			c = '.';
		printf("%c", c);
	}
	for (; i < bytesPerLine; i++)
		printf(" ");
	printf("|\n");
}

#pragma mark -


RandomSource::RandomSource() {
	// Use system time as RNG seed. Normally not a good idea, if you are using
	// a RNG for security purposes, but good enough for our purposes.
	assert(g_driver);
	uint32 seed = g_driver->getMillis();
	setSeed(seed);
}

void RandomSource::setSeed(uint32 seed) {
	_randSeed = seed;
}

uint RandomSource::getRandomNumber(uint max) {
	_randSeed = 0xDEADBF03 * (_randSeed + 1);
	_randSeed = (_randSeed >> 13) | (_randSeed << 19);
	return _randSeed % (max + 1);
}

uint RandomSource::getRandomBit(void) {
	_randSeed = 0xDEADBF03 * (_randSeed + 1);
	_randSeed = (_randSeed >> 13) | (_randSeed << 19);
	return _randSeed & 1;
}

uint RandomSource::getRandomNumberRng(uint min, uint max) {
	return getRandomNumber(max - min) + min;
}

Common::String tag2string(uint32 tag) {
	char str[5];
	str[0] = (char)(tag >> 24);
	str[1] = (char)(tag >> 16);
	str[2] = (char)(tag >> 8);
	str[3] = (char)tag;
	str[4] = '\0';
	return Common::String(str);
}

}	// End of namespace Common

