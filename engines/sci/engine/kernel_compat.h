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
 * $URL$
 * $Id$
 *
 */

/* Kernel compatibility #defines to pre-GLUTTON times */

#ifndef _SCI_KERNEL_COMPAT_
#define _SCI_KERNEL_COMPAT_

#ifdef __GNUC__XX
#warning "Old kernel compatibility crap"
#endif

namespace Sci {

// Minimal heap position/
#define HEAP_MIN 800

#define GET_HEAP(address) ((((guint16)(address)) < HEAP_MIN) ? \
			KERNEL_OOPS("Heap address space violation on read")  \
			: getHeapInt16(s->heap, ((guint16)(address))))
// Reads a heap value if allowed

#define UGET_HEAP(address) ((((guint16)(address)) < HEAP_MIN)? \
			KERNEL_OOPS("Heap address space violation on read")  \
			: getHeapUInt16(s->heap, ((guint16)(address))))
// Reads a heap value if allowed

#define PUT_HEAP(address, value) { \
			if (((guint16)(address)) < HEAP_MIN) \
				KERNEL_OOPS("Heap address space violation on write");        \
			else { \
				s->heap[((guint16)(address))] = (value) &0xff;               \
				s->heap[((guint16)(address)) + 1] = ((value) >> 8) & 0xff;}    \
				if ((address) & 1)                                                 \
					sciprintf("Warning: Unaligned write to %04x\n", (address) & 0xffff); \
			} \
}
// Sets a heap value if allowed

static inline int getHeapInt16(unsigned char *base, int address) {
	if (address & 1)
		sciprintf("Warning: Unaligned read from %04x\n", (address) & 0xffff);

	return getInt16(base + address);
}

static inline unsigned int getHeapUInt16(unsigned char *base, int address) {
	if (address & 1)
		sciprintf("Warning: Unaligned unsigned read from %04x\n", (address) & 0xffff);

	return getUInt16(base + address);
}

} // End of namespace Sci

#endif // !_SCI_KERNEL_COMPAT_
