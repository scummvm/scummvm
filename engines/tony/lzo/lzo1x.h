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
/* lzo1x.h -- public interface of the LZO1X compression algorithm

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1999 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1998 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1997 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996 Markus Franz Xaver Johannes Oberhumer

   The LZO library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   The LZO library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the LZO library; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   Markus F.X.J. Oberhumer
   <markus.oberhumer@jk.uni-linz.ac.at>
   http://wildsau.idv.uni-linz.ac.at/mfx/lzo.html
 */


#ifndef __LZO1X_H
#define __LZO1X_H

#ifndef __LZOCONF_H
#include "lzoconf.h"
#endif

namespace Tony {

namespace LZO {

#ifdef __cplusplus
extern "C" {
#endif


/***********************************************************************
//
************************************************************************/

/* Memory required for the wrkmem parameter.
 * When the required size is 0, you can also pass a NULL pointer.
 */

#define LZO1X_MEM_COMPRESS      LZO1X_1_MEM_COMPRESS
#define LZO1X_MEM_DECOMPRESS    (0)
#define LZO1X_MEM_OPTIMIZE      (0)


/* decompression */
LZO_EXTERN(int)
lzo1x_decompress        ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uint *dst_len,
                                lzo_voidp wrkmem /* NOT USED */ );

/* safe decompression with overrun testing */
LZO_EXTERN(int)
lzo1x_decompress_safe   ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uint *dst_len,
                                lzo_voidp wrkmem /* NOT USED */ );


/***********************************************************************
//
************************************************************************/

#define LZO1X_1_MEM_COMPRESS    ((lzo_uint32) (16384L * lzo_sizeof_dict_t))

LZO_EXTERN(int)
lzo1x_1_compress        ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uint *dst_len,
                                lzo_voidp wrkmem );


/***********************************************************************
// special compressor versions
************************************************************************/

/* this version needs only 8 kB work memory */
#define LZO1X_1_11_MEM_COMPRESS ((lzo_uint32) (2048L * lzo_sizeof_dict_t))

LZO_EXTERN(int)
lzo1x_1_11_compress     ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uint *dst_len,
                                lzo_voidp wrkmem );


/* this version needs 16 kB work memory */
#define LZO1X_1_12_MEM_COMPRESS ((lzo_uint32) (4096L * lzo_sizeof_dict_t))

LZO_EXTERN(int)
lzo1x_1_12_compress     ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uint *dst_len,
                                lzo_voidp wrkmem );


/* use this version if you need a little more compression speed */
#define LZO1X_1_15_MEM_COMPRESS ((lzo_uint32) (32768L * lzo_sizeof_dict_t))

LZO_EXTERN(int)
lzo1x_1_15_compress     ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uint *dst_len,
                                lzo_voidp wrkmem );


/***********************************************************************
// better compression ratio at the cost of more memory and time
************************************************************************/

#define LZO1X_999_MEM_COMPRESS  ((lzo_uint32) (14 * 16384L * sizeof(short)))

#if !defined(LZO_999_UNSUPPORTED)
LZO_EXTERN(int)
lzo1x_999_compress      ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uint *dst_len,
                                lzo_voidp wrkmem );
#endif


/***********************************************************************
//
************************************************************************/

#if !defined(LZO_999_UNSUPPORTED)
LZO_EXTERN(int)
lzo1x_999_compress_dict     ( const lzo_byte *in , lzo_uint  in_len,
                                    lzo_byte *out, lzo_uint *out_len,
                                    lzo_voidp wrkmem,
                              const lzo_byte *dict, lzo_uint dict_len );

LZO_EXTERN(int)
lzo1x_999_compress_level    ( const lzo_byte *in , lzo_uint  in_len,
                                    lzo_byte *out, lzo_uint *out_len,
                                    lzo_voidp wrkmem,
                              const lzo_byte *dict, lzo_uint dict_len,
                                    lzo_progress_callback_t cb,
                                    int compression_level );
#endif

LZO_EXTERN(int)
lzo1x_decompress_dict_safe ( const lzo_byte *in,  lzo_uint  in_len,
                                   lzo_byte *out, lzo_uint *out_len,
                                   lzo_voidp wrkmem /* NOT USED */,
                             const lzo_byte *dict, lzo_uint dict_len );


/***********************************************************************
// optimize a compressed data block
************************************************************************/

LZO_EXTERN(int)
lzo1x_optimize          (       lzo_byte *in , lzo_uint  in_len,
                                lzo_byte *out, lzo_uint *out_len,
                                lzo_voidp wrkmem );



#ifdef __cplusplus
} /* extern "C" */
#endif

} // End of namespace LZO

} // End of namespace Tony

#endif /* already included */

