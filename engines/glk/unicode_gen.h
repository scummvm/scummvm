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
 */

#ifndef GLK_UNICODE_GEN_H
#define GLK_UNICODE_GEN_H

#include "glk/glk_types.h"

namespace Glk {

#define GET_CASE_BLOCK(ch, blockptr)  \
	switch ((uint)(ch) >> 8) {  \
	case 0x0:  \
		*blockptr = unigen_case_block_0x0;  \
		break;  \
	case 0x1:  \
		*blockptr = unigen_case_block_0x1;  \
		break;  \
	case 0x2:  \
		*blockptr = unigen_case_block_0x2;  \
		break;  \
	case 0x3:  \
		*blockptr = unigen_case_block_0x3;  \
		break;  \
	case 0x4:  \
		*blockptr = unigen_case_block_0x4;  \
		break;  \
	case 0x5:  \
		*blockptr = unigen_case_block_0x5;  \
		break;  \
	case 0x1e:  \
		*blockptr = unigen_case_block_0x1e;  \
		break;  \
	case 0x1f:  \
		*blockptr = unigen_case_block_0x1f;  \
		break;  \
	case 0x21:  \
		*blockptr = unigen_case_block_0x21;  \
		break;  \
	case 0x24:  \
		*blockptr = unigen_case_block_0x24;  \
		break;  \
	case 0xfb:  \
		*blockptr = unigen_case_block_0xfb;  \
		break;  \
	case 0xff:  \
		*blockptr = unigen_case_block_0xff;  \
		break;  \
	case 0x104:  \
		*blockptr = unigen_case_block_0x104;  \
		break;  \
	default:  \
		*blockptr = nullptr;  \
	}

#define GET_CASE_SPECIAL(ch, specptr)  \
	switch (ch) {  \
	case 0xdf:  \
		*specptr = unigen_special_0xdf;  \
		break;  \
	case 0x130:  \
		*specptr = unigen_special_0x130;  \
		break;  \
	case 0x149:  \
		*specptr = unigen_special_0x149;  \
		break;  \
	case 0x1c4:  \
		*specptr = unigen_special_0x1c4;  \
		break;  \
	case 0x1c5:  \
		*specptr = unigen_special_0x1c5;  \
		break;  \
	case 0x1c6:  \
		*specptr = unigen_special_0x1c6;  \
		break;  \
	case 0x1c7:  \
		*specptr = unigen_special_0x1c7;  \
		break;  \
	case 0x1c8:  \
		*specptr = unigen_special_0x1c8;  \
		break;  \
	case 0x1c9:  \
		*specptr = unigen_special_0x1c9;  \
		break;  \
	case 0x1ca:  \
		*specptr = unigen_special_0x1ca;  \
		break;  \
	case 0x1cb:  \
		*specptr = unigen_special_0x1cb;  \
		break;  \
	case 0x1cc:  \
		*specptr = unigen_special_0x1cc;  \
		break;  \
	case 0x1f0:  \
		*specptr = unigen_special_0x1f0;  \
		break;  \
	case 0x1f1:  \
		*specptr = unigen_special_0x1f1;  \
		break;  \
	case 0x1f2:  \
		*specptr = unigen_special_0x1f2;  \
		break;  \
	case 0x1f3:  \
		*specptr = unigen_special_0x1f3;  \
		break;  \
	case 0x390:  \
		*specptr = unigen_special_0x390;  \
		break;  \
	case 0x3b0:  \
		*specptr = unigen_special_0x3b0;  \
		break;  \
	case 0x587:  \
		*specptr = unigen_special_0x587;  \
		break;  \
	case 0x1e96:  \
		*specptr = unigen_special_0x1e96;  \
		break;  \
	case 0x1e97:  \
		*specptr = unigen_special_0x1e97;  \
		break;  \
	case 0x1e98:  \
		*specptr = unigen_special_0x1e98;  \
		break;  \
	case 0x1e99:  \
		*specptr = unigen_special_0x1e99;  \
		break;  \
	case 0x1e9a:  \
		*specptr = unigen_special_0x1e9a;  \
		break;  \
	case 0x1f50:  \
		*specptr = unigen_special_0x1f50;  \
		break;  \
	case 0x1f52:  \
		*specptr = unigen_special_0x1f52;  \
		break;  \
	case 0x1f54:  \
		*specptr = unigen_special_0x1f54;  \
		break;  \
	case 0x1f56:  \
		*specptr = unigen_special_0x1f56;  \
		break;  \
	case 0x1f80:  \
		*specptr = unigen_special_0x1f80;  \
		break;  \
	case 0x1f81:  \
		*specptr = unigen_special_0x1f81;  \
		break;  \
	case 0x1f82:  \
		*specptr = unigen_special_0x1f82;  \
		break;  \
	case 0x1f83:  \
		*specptr = unigen_special_0x1f83;  \
		break;  \
	case 0x1f84:  \
		*specptr = unigen_special_0x1f84;  \
		break;  \
	case 0x1f85:  \
		*specptr = unigen_special_0x1f85;  \
		break;  \
	case 0x1f86:  \
		*specptr = unigen_special_0x1f86;  \
		break;  \
	case 0x1f87:  \
		*specptr = unigen_special_0x1f87;  \
		break;  \
	case 0x1f88:  \
		*specptr = unigen_special_0x1f88;  \
		break;  \
	case 0x1f89:  \
		*specptr = unigen_special_0x1f89;  \
		break;  \
	case 0x1f8a:  \
		*specptr = unigen_special_0x1f8a;  \
		break;  \
	case 0x1f8b:  \
		*specptr = unigen_special_0x1f8b;  \
		break;  \
	case 0x1f8c:  \
		*specptr = unigen_special_0x1f8c;  \
		break;  \
	case 0x1f8d:  \
		*specptr = unigen_special_0x1f8d;  \
		break;  \
	case 0x1f8e:  \
		*specptr = unigen_special_0x1f8e;  \
		break;  \
	case 0x1f8f:  \
		*specptr = unigen_special_0x1f8f;  \
		break;  \
	case 0x1f90:  \
		*specptr = unigen_special_0x1f90;  \
		break;  \
	case 0x1f91:  \
		*specptr = unigen_special_0x1f91;  \
		break;  \
	case 0x1f92:  \
		*specptr = unigen_special_0x1f92;  \
		break;  \
	case 0x1f93:  \
		*specptr = unigen_special_0x1f93;  \
		break;  \
	case 0x1f94:  \
		*specptr = unigen_special_0x1f94;  \
		break;  \
	case 0x1f95:  \
		*specptr = unigen_special_0x1f95;  \
		break;  \
	case 0x1f96:  \
		*specptr = unigen_special_0x1f96;  \
		break;  \
	case 0x1f97:  \
		*specptr = unigen_special_0x1f97;  \
		break;  \
	case 0x1f98:  \
		*specptr = unigen_special_0x1f98;  \
		break;  \
	case 0x1f99:  \
		*specptr = unigen_special_0x1f99;  \
		break;  \
	case 0x1f9a:  \
		*specptr = unigen_special_0x1f9a;  \
		break;  \
	case 0x1f9b:  \
		*specptr = unigen_special_0x1f9b;  \
		break;  \
	case 0x1f9c:  \
		*specptr = unigen_special_0x1f9c;  \
		break;  \
	case 0x1f9d:  \
		*specptr = unigen_special_0x1f9d;  \
		break;  \
	case 0x1f9e:  \
		*specptr = unigen_special_0x1f9e;  \
		break;  \
	case 0x1f9f:  \
		*specptr = unigen_special_0x1f9f;  \
		break;  \
	case 0x1fa0:  \
		*specptr = unigen_special_0x1fa0;  \
		break;  \
	case 0x1fa1:  \
		*specptr = unigen_special_0x1fa1;  \
		break;  \
	case 0x1fa2:  \
		*specptr = unigen_special_0x1fa2;  \
		break;  \
	case 0x1fa3:  \
		*specptr = unigen_special_0x1fa3;  \
		break;  \
	case 0x1fa4:  \
		*specptr = unigen_special_0x1fa4;  \
		break;  \
	case 0x1fa5:  \
		*specptr = unigen_special_0x1fa5;  \
		break;  \
	case 0x1fa6:  \
		*specptr = unigen_special_0x1fa6;  \
		break;  \
	case 0x1fa7:  \
		*specptr = unigen_special_0x1fa7;  \
		break;  \
	case 0x1fa8:  \
		*specptr = unigen_special_0x1fa8;  \
		break;  \
	case 0x1fa9:  \
		*specptr = unigen_special_0x1fa9;  \
		break;  \
	case 0x1faa:  \
		*specptr = unigen_special_0x1faa;  \
		break;  \
	case 0x1fab:  \
		*specptr = unigen_special_0x1fab;  \
		break;  \
	case 0x1fac:  \
		*specptr = unigen_special_0x1fac;  \
		break;  \
	case 0x1fad:  \
		*specptr = unigen_special_0x1fad;  \
		break;  \
	case 0x1fae:  \
		*specptr = unigen_special_0x1fae;  \
		break;  \
	case 0x1faf:  \
		*specptr = unigen_special_0x1faf;  \
		break;  \
	case 0x1fb2:  \
		*specptr = unigen_special_0x1fb2;  \
		break;  \
	case 0x1fb3:  \
		*specptr = unigen_special_0x1fb3;  \
		break;  \
	case 0x1fb4:  \
		*specptr = unigen_special_0x1fb4;  \
		break;  \
	case 0x1fb6:  \
		*specptr = unigen_special_0x1fb6;  \
		break;  \
	case 0x1fb7:  \
		*specptr = unigen_special_0x1fb7;  \
		break;  \
	case 0x1fbc:  \
		*specptr = unigen_special_0x1fbc;  \
		break;  \
	case 0x1fc2:  \
		*specptr = unigen_special_0x1fc2;  \
		break;  \
	case 0x1fc3:  \
		*specptr = unigen_special_0x1fc3;  \
		break;  \
	case 0x1fc4:  \
		*specptr = unigen_special_0x1fc4;  \
		break;  \
	case 0x1fc6:  \
		*specptr = unigen_special_0x1fc6;  \
		break;  \
	case 0x1fc7:  \
		*specptr = unigen_special_0x1fc7;  \
		break;  \
	case 0x1fcc:  \
		*specptr = unigen_special_0x1fcc;  \
		break;  \
	case 0x1fd2:  \
		*specptr = unigen_special_0x1fd2;  \
		break;  \
	case 0x1fd3:  \
		*specptr = unigen_special_0x1fd3;  \
		break;  \
	case 0x1fd6:  \
		*specptr = unigen_special_0x1fd6;  \
		break;  \
	case 0x1fd7:  \
		*specptr = unigen_special_0x1fd7;  \
		break;  \
	case 0x1fe2:  \
		*specptr = unigen_special_0x1fe2;  \
		break;  \
	case 0x1fe3:  \
		*specptr = unigen_special_0x1fe3;  \
		break;  \
	case 0x1fe4:  \
		*specptr = unigen_special_0x1fe4;  \
		break;  \
	case 0x1fe6:  \
		*specptr = unigen_special_0x1fe6;  \
		break;  \
	case 0x1fe7:  \
		*specptr = unigen_special_0x1fe7;  \
		break;  \
	case 0x1ff2:  \
		*specptr = unigen_special_0x1ff2;  \
		break;  \
	case 0x1ff3:  \
		*specptr = unigen_special_0x1ff3;  \
		break;  \
	case 0x1ff4:  \
		*specptr = unigen_special_0x1ff4;  \
		break;  \
	case 0x1ff6:  \
		*specptr = unigen_special_0x1ff6;  \
		break;  \
	case 0x1ff7:  \
		*specptr = unigen_special_0x1ff7;  \
		break;  \
	case 0x1ffc:  \
		*specptr = unigen_special_0x1ffc;  \
		break;  \
	case 0xfb00:  \
		*specptr = unigen_special_0xfb00;  \
		break;  \
	case 0xfb01:  \
		*specptr = unigen_special_0xfb01;  \
		break;  \
	case 0xfb02:  \
		*specptr = unigen_special_0xfb02;  \
		break;  \
	case 0xfb03:  \
		*specptr = unigen_special_0xfb03;  \
		break;  \
	case 0xfb04:  \
		*specptr = unigen_special_0xfb04;  \
		break;  \
	case 0xfb05:  \
		*specptr = unigen_special_0xfb05;  \
		break;  \
	case 0xfb06:  \
		*specptr = unigen_special_0xfb06;  \
		break;  \
	case 0xfb13:  \
		*specptr = unigen_special_0xfb13;  \
		break;  \
	case 0xfb14:  \
		*specptr = unigen_special_0xfb14;  \
		break;  \
	case 0xfb15:  \
		*specptr = unigen_special_0xfb15;  \
		break;  \
	case 0xfb16:  \
		*specptr = unigen_special_0xfb16;  \
		break;  \
	case 0xfb17:  \
		*specptr = unigen_special_0xfb17;  \
		break;  \
	default:  \
		*specptr = nullptr;  \
	}

typedef uint gli_case_block_t[2];   ///< upper, lower
typedef uint gli_case_special_t[3]; ///< upper, lower, title
typedef uint gli_decomp_block_t[2]; ///< count, position

extern gli_case_block_t unigen_case_block_0x0[256];
extern gli_case_block_t unigen_case_block_0x1[256];
extern gli_case_block_t unigen_case_block_0x2[256];
extern gli_case_block_t unigen_case_block_0x3[256];
extern gli_case_block_t unigen_case_block_0x4[256];
extern gli_case_block_t unigen_case_block_0x5[256];
extern gli_case_block_t unigen_case_block_0x1e[256];
extern gli_case_block_t unigen_case_block_0x1f[256];
extern gli_case_block_t unigen_case_block_0x21[256];
extern gli_case_block_t unigen_case_block_0x24[256];
extern gli_case_block_t unigen_case_block_0xfb[256];
extern gli_case_block_t unigen_case_block_0xff[256];
extern gli_case_block_t unigen_case_block_0x104[256];

extern gli_case_special_t unigen_special_0xdf;
extern gli_case_special_t unigen_special_0x130;
extern gli_case_special_t unigen_special_0x149;
extern gli_case_special_t unigen_special_0x1c4;
extern gli_case_special_t unigen_special_0x1c5;
extern gli_case_special_t unigen_special_0x1c6;
extern gli_case_special_t unigen_special_0x1c7;
extern gli_case_special_t unigen_special_0x1c8;
extern gli_case_special_t unigen_special_0x1c9;
extern gli_case_special_t unigen_special_0x1ca;
extern gli_case_special_t unigen_special_0x1cb;
extern gli_case_special_t unigen_special_0x1cc;
extern gli_case_special_t unigen_special_0x1f0;
extern gli_case_special_t unigen_special_0x1f1;
extern gli_case_special_t unigen_special_0x1f2;
extern gli_case_special_t unigen_special_0x1f3;
extern gli_case_special_t unigen_special_0x390;
extern gli_case_special_t unigen_special_0x3b0;
extern gli_case_special_t unigen_special_0x587;
extern gli_case_special_t unigen_special_0x1e96;
extern gli_case_special_t unigen_special_0x1e97;
extern gli_case_special_t unigen_special_0x1e98;
extern gli_case_special_t unigen_special_0x1e99;
extern gli_case_special_t unigen_special_0x1e9a;
extern gli_case_special_t unigen_special_0x1f50;
extern gli_case_special_t unigen_special_0x1f52;
extern gli_case_special_t unigen_special_0x1f54;
extern gli_case_special_t unigen_special_0x1f56;
extern gli_case_special_t unigen_special_0x1f80;
extern gli_case_special_t unigen_special_0x1f81;
extern gli_case_special_t unigen_special_0x1f82;
extern gli_case_special_t unigen_special_0x1f83;
extern gli_case_special_t unigen_special_0x1f84;
extern gli_case_special_t unigen_special_0x1f85;
extern gli_case_special_t unigen_special_0x1f86;
extern gli_case_special_t unigen_special_0x1f87;
extern gli_case_special_t unigen_special_0x1f88;
extern gli_case_special_t unigen_special_0x1f89;
extern gli_case_special_t unigen_special_0x1f8a;
extern gli_case_special_t unigen_special_0x1f8b;
extern gli_case_special_t unigen_special_0x1f8c;
extern gli_case_special_t unigen_special_0x1f8d;
extern gli_case_special_t unigen_special_0x1f8e;
extern gli_case_special_t unigen_special_0x1f8f;
extern gli_case_special_t unigen_special_0x1f90;
extern gli_case_special_t unigen_special_0x1f91;
extern gli_case_special_t unigen_special_0x1f92;
extern gli_case_special_t unigen_special_0x1f93;
extern gli_case_special_t unigen_special_0x1f94;
extern gli_case_special_t unigen_special_0x1f95;
extern gli_case_special_t unigen_special_0x1f96;
extern gli_case_special_t unigen_special_0x1f97;
extern gli_case_special_t unigen_special_0x1f98;
extern gli_case_special_t unigen_special_0x1f99;
extern gli_case_special_t unigen_special_0x1f9a;
extern gli_case_special_t unigen_special_0x1f9b;
extern gli_case_special_t unigen_special_0x1f9c;
extern gli_case_special_t unigen_special_0x1f9d;
extern gli_case_special_t unigen_special_0x1f9e;
extern gli_case_special_t unigen_special_0x1f9f;
extern gli_case_special_t unigen_special_0x1fa0;
extern gli_case_special_t unigen_special_0x1fa1;
extern gli_case_special_t unigen_special_0x1fa2;
extern gli_case_special_t unigen_special_0x1fa3;
extern gli_case_special_t unigen_special_0x1fa4;
extern gli_case_special_t unigen_special_0x1fa5;
extern gli_case_special_t unigen_special_0x1fa6;
extern gli_case_special_t unigen_special_0x1fa7;
extern gli_case_special_t unigen_special_0x1fa8;
extern gli_case_special_t unigen_special_0x1fa9;
extern gli_case_special_t unigen_special_0x1faa;
extern gli_case_special_t unigen_special_0x1fab;
extern gli_case_special_t unigen_special_0x1fac;
extern gli_case_special_t unigen_special_0x1fad;
extern gli_case_special_t unigen_special_0x1fae;
extern gli_case_special_t unigen_special_0x1faf;
extern gli_case_special_t unigen_special_0x1fb2;
extern gli_case_special_t unigen_special_0x1fb3;
extern gli_case_special_t unigen_special_0x1fb4;
extern gli_case_special_t unigen_special_0x1fb6;
extern gli_case_special_t unigen_special_0x1fb7;
extern gli_case_special_t unigen_special_0x1fbc;
extern gli_case_special_t unigen_special_0x1fc2;
extern gli_case_special_t unigen_special_0x1fc3;
extern gli_case_special_t unigen_special_0x1fc4;
extern gli_case_special_t unigen_special_0x1fc6;
extern gli_case_special_t unigen_special_0x1fc7;
extern gli_case_special_t unigen_special_0x1fcc;
extern gli_case_special_t unigen_special_0x1fd2;
extern gli_case_special_t unigen_special_0x1fd3;
extern gli_case_special_t unigen_special_0x1fd6;
extern gli_case_special_t unigen_special_0x1fd7;
extern gli_case_special_t unigen_special_0x1fe2;
extern gli_case_special_t unigen_special_0x1fe3;
extern gli_case_special_t unigen_special_0x1fe4;
extern gli_case_special_t unigen_special_0x1fe6;
extern gli_case_special_t unigen_special_0x1fe7;
extern gli_case_special_t unigen_special_0x1ff2;
extern gli_case_special_t unigen_special_0x1ff3;
extern gli_case_special_t unigen_special_0x1ff4;
extern gli_case_special_t unigen_special_0x1ff6;
extern gli_case_special_t unigen_special_0x1ff7;
extern gli_case_special_t unigen_special_0x1ffc;
extern gli_case_special_t unigen_special_0xfb00;
extern gli_case_special_t unigen_special_0xfb01;
extern gli_case_special_t unigen_special_0xfb02;
extern gli_case_special_t unigen_special_0xfb03;
extern gli_case_special_t unigen_special_0xfb04;
extern gli_case_special_t unigen_special_0xfb05;
extern gli_case_special_t unigen_special_0xfb06;
extern gli_case_special_t unigen_special_0xfb13;
extern gli_case_special_t unigen_special_0xfb14;
extern gli_case_special_t unigen_special_0xfb15;
extern gli_case_special_t unigen_special_0xfb16;
extern gli_case_special_t unigen_special_0xfb17;
extern uint unigen_special_array[];

} // End of namespace Glk

#endif
