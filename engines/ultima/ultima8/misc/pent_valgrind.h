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

#ifndef PENT_VALGRIND_H
#define PENT_VALGRIND_H

#ifdef USE_VALGRIND

#include <valgrind/memcheck.h>

#else

#define VALGRIND_MAKE_MEM_NOACCESS(_qzz_addr,_qzz_len)
#define VALGRIND_MAKE_MEM_UNDEFINED(_qzz_addr,_qzz_len)
#define VALGRIND_MAKE_MEM_DEFINED(_qzz_addr,_qzz_len)

#define VALGRIND_CHECK_MEM_IS_DEFINED(_qzz_addr,_qzz_len)
#define VALGRIND_CHECK_VALUE_IS_DEFINED(__lvalue)
#define VALGRIND_CHECK_MEM_IS_ADDRESSABLE(_qzz_addr,_qzz_len)

#define VALGRIND_CREATE_MEMPOOL(pool,rzB,is_zeroed)
#define VALGRIND_DESTROY_MEMPOOL(pool)
#define VALGRIND_MEMPOOL_ALLOC(pool,addr,size)
#define VALGRIND_MEMPOOL_FREE(pool,addr)

#define VALGRIND_CREATE_BLOCK(_qzz_addr,_qzz_len,_qzz_desc) 0
#define VALGRIND_DISCARD(_qzz_blkindex)

#endif

#endif
