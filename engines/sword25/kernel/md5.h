// -----------------------------------------------------------------------------
// This file is part of Broken Sword 2.5
// Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdörfer
//
// Broken Sword 2.5 is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Broken Sword 2.5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Broken Sword 2.5; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
// -----------------------------------------------------------------------------

#ifndef SWORD25_MD5_H
#define SWORD25_MD5_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include "sword25/kernel/bs_stdint.h"

// -----------------------------------------------------------------------------
// Klassendefinition
// -----------------------------------------------------------------------------

typedef uint8_t md5_byte_t; /* 8-bit byte */
typedef uint32_t md5_word_t; /* 32-bit word */

/* Define the state of the MD5 Algorithm. */
typedef struct md5_state_s {
	md5_word_t count[2];	/* message length in bits, lsw first */
	md5_word_t abcd[4];		/* digest buffer */
	md5_byte_t buf[64];		/* accumulate block */
} md5_state_t;

class BS_MD5
{
public:
	BS_MD5();

	void Update(const unsigned char * Buffer, unsigned int Length);
	void GetDigest(unsigned char Digest[16]);

private:
	md5_state_t m_state;
};

#endif
