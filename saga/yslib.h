/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
#ifndef YSLIB_MAIN_H__
#define YSLIB_MAIN_H__

namespace Saga {

enum YS_ERROR_STATES {

	YS_E_SUCCESS = 0,
	YS_E_FAILURE,
	YS_E_MEM
};

enum YS_CONFIRM_STATES {

	YS_CONFIRM_NO = 0,
	YS_CONFIRM_YES,
	YS_CONFIRM_CANCEL
};

/* General purpose quantity-comparison function */
typedef int (YS_COMPARE_FUNC) (const void *, const void *);

/* General-purpose utility macros 
\*------------------------------------------------------------------*/

/* Ignore a parameter (Supress warnings) */
#define YS_IGNORE_PARAM( param ) ( void )( param )

/* ys_binread.c : Binary input functions (buffer oriented)
\*------------------------------------------------------------------*/

/* #define YS_ASSUME_2S_COMP */

/* Read a 4CC ( Four characater code ) */
void
ys_read_4cc(char *fourcc,
    const unsigned char *data_p, const unsigned char **data_pp);

/* Read 8 bit unsigned integer */
unsigned int ys_read_u8(const unsigned char *, const unsigned char **);

/* Read 8 bit signed integer */
int ys_read_s8(const unsigned char *, const unsigned char **);

/* Read 16 bit unsigned integer, big-endian */
unsigned int ys_read_u16_be(const unsigned char *, const unsigned char **);

/* Read 16 bit unsigned integer, little-endian */
unsigned int ys_read_u16_le(const unsigned char *, const unsigned char **);

/* Read 16 bit signed integer, 2's complement, big-endian */
int ys_read_s16_be(const unsigned char *, const unsigned char **);

/* Read 16 bit signed integer, 2's complement, little-endian */
int ys_read_s16_le(const unsigned char *, const unsigned char **);

/* Read 24 bit unsigned integer, big-endian */
unsigned long ys_read_u24_be(const unsigned char *, const unsigned char **);

/* Read 24 bit unsigned integer, little-endian */
unsigned long ys_read_u24_le(const unsigned char *, const unsigned char **);

/* Read 24 bit signed integer, 2's complement, big-endian */
long ys_read_s24_be(const unsigned char *, const unsigned char **);

/* Read 24 bit signed integer, 2's complement, little-endian */
long ys_read_s24_le(const unsigned char *, const unsigned char **);

/* Read 32 bit unsigned integer, big-endian */
unsigned long ys_read_u32_be(const unsigned char *, const unsigned char **);

/* Read 32 bit unsigned integer, little-endian */
unsigned long ys_read_u32_le(const unsigned char *, const unsigned char **);

/* Read 32 bit signed integer, 2's complement, big-endian */
long ys_read_s32_be(const unsigned char *, const unsigned char **);

/* Read 32 bit signed integer, 2's complement, little-endian */
long ys_read_s32_le(const unsigned char *, const unsigned char **);

/* ys_binwrite.c : Binary output functions ( buffer oriented ) 
\*------------------------------------------------------------------*/

void ys_write_u8(unsigned int, unsigned char *, unsigned char **);

/* Write 16 bit unsigned integer, big-endian */
void ys_write_u16_be(unsigned int, unsigned char *, unsigned char **);

/* Write 16 bit unsigned integer, little-endian */
void ys_write_u16_le(unsigned int, unsigned char *, unsigned char **);

/* Write 16 bit signed integer, 2's complement, big-endian */
void ys_write_s16_be(int, unsigned char *, unsigned char **);

/* Write 16 bit signed integer, 2's complement, little-endian */
void ys_write_s16_le(int, unsigned char *, unsigned char **);

/* Write 24 bit unsigned integer, big-endian */
void ys_write_u24_be(unsigned long, unsigned char *, unsigned char **);

/* Write 24 bit unsigned integer, little-endian */
void ys_write_u24_le(unsigned long, unsigned char *, unsigned char **);

/* Write 24 bit signed integer, 2's complement, big-endian */
void ys_write_s24_be(long, unsigned char *, unsigned char **);

/* Write 24 bit signed integer, 2's complement, little-endian */
void ys_write_s24_le(long, unsigned char *, unsigned char **);

/* Write 32 bit unsigned integer, big-endian */
void ys_write_u32_be(unsigned long, unsigned char *, unsigned char **);

/* Write 32 bit unsigned integer, little-endian */
void ys_write_u32_le(unsigned long, unsigned char *, unsigned char **);

/* Write 32 bit signed integer, 2's complement, big-endian */
void ys_write_s32_be(long, unsigned char *, unsigned char **);

/* Write 32 bit signed integer, 2's complement, little-endian */
void ys_write_s32_le(long, unsigned char *, unsigned char **);


/* Shared declarations for list modules
\*------------------------------------------------------------------*/
enum YS_WALK_DIRECTIONS {
	YS_WALK_BACKWARD = 0,
	YS_WALK_FORWARD
};

/* ys_dl_list.c  : Doubly-linked list functions
\*------------------------------------------------------------------*/
typedef struct ys_dl_node_tag YS_DL_NODE;
typedef struct ys_dl_node_tag YS_DL_LIST;

struct ys_dl_node_tag {
	void *data;
	struct ys_dl_node_tag *next;
	struct ys_dl_node_tag *prev;
};

YS_DL_LIST *ys_dll_create(void);
void ys_dll_destroy(YS_DL_LIST *);

void *ys_dll_get_data(YS_DL_NODE *);

YS_DL_NODE *ys_dll_head(YS_DL_LIST *);
YS_DL_NODE *ys_dll_tail(YS_DL_LIST *);
YS_DL_NODE *ys_dll_next(YS_DL_NODE *);
YS_DL_NODE *ys_dll_prev(YS_DL_NODE *);

YS_DL_NODE *ys_dll_add_head(YS_DL_LIST *, void *, size_t);
YS_DL_NODE *ys_dll_add_tail(YS_DL_LIST *, void *, size_t);

#define ys_dll_preinsert  ys_dll_add_tail
#define ys_dll_postinsert ys_dll_add_head

YS_DL_NODE *ys_dll_insert(YS_DL_LIST *, void *, size_t, YS_COMPARE_FUNC *);
int ys_dll_delete(YS_DL_NODE *);
void ys_dll_delete_all(YS_DL_LIST *);

YS_DL_NODE *ys_dll_replace(YS_DL_NODE *, void *, size_t);

int ys_dll_reorder_up(YS_DL_LIST *, YS_DL_NODE *, YS_COMPARE_FUNC *);
int ys_dll_reorder_down(YS_DL_LIST *, YS_DL_NODE *, YS_COMPARE_FUNC *);

int ys_dll_foreach(YS_DL_LIST *,
    int, void *, int (*)(void *, void *), YS_DL_NODE **);


} // End of namespace Saga

#endif				/* YSLIB_MAIN_H__ */
