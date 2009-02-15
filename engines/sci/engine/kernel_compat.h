/***************************************************************************
 kernel_compat.h Copyright (C) 2002 Christoph Reichenbach


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Christoph Reichenbach (CR) <jameson@linuxgames.com>

***************************************************************************/
/* Kernel compatibility #defines to pre-GLUTTON times */

#ifndef _SCI_KERNEL_COMPAT_
#define _SCI_KERNEL_COMPAT_

#ifdef __GNUC__XX
#warning "Old kernel compatibility crap"
#endif

/* Minimal heap position */
#define HEAP_MIN 800

#define GET_HEAP(address) ((((guint16)(address)) < HEAP_MIN)? \
KERNEL_OOPS("Heap address space violation on read")  \
: getHeapInt16(s->heap, ((guint16)(address))))
/* Reads a heap value if allowed */

#define UGET_HEAP(address) ((((guint16)(address)) < HEAP_MIN)? \
KERNEL_OOPS("Heap address space violation on read")  \
: getHeapUInt16(s->heap, ((guint16)(address))))
/* Reads a heap value if allowed */

#define PUT_HEAP(address, value) { if (((guint16)(address)) < HEAP_MIN) \
KERNEL_OOPS("Heap address space violation on write");        \
else { s->heap[((guint16)(address))] = (value) &0xff;               \
 s->heap[((guint16)(address)) + 1] = ((value) >> 8) & 0xff;}    \
if ((address) & 1)                                                 \
  sciprintf("Warning: Unaligned write to %04x\n", (address) & 0xffff); }
/* Sets a heap value if allowed */

static inline int
getHeapInt16(unsigned char *base, int address) {
	if (address & 1)
		sciprintf("Warning: Unaligned read from %04x\n", (address) & 0xffff);

	return getInt16(base + address);
}

static inline unsigned int
getHeapUInt16(unsigned char *base, int address) {
	if (address & 1)
		sciprintf("Warning: Unaligned unsigned read from %04x\n", (address) & 0xffff);

	return getUInt16(base + address);
}



#endif /* !_SCI_KERNEL_COMPAT_ */
