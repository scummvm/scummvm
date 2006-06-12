/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
 * Copyright (C) 2002 Ph0x - GP32 Backend
 * Copyright (C) 2003/2004 DJWillis - GP32 Backend
 * Copyright (C) 2005 Won Star - GP32 Backend
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

#ifndef __GP32STD_H
#define __GP32STD_H

#include <gpfont.h>
#include <gpfont_port.h>
#include <gpgraphic.h>
#include <gpmm.h>
#include <gpmem.h>
#include <gpos_def.h>
#include <gpstdio.h>
#include <gpstdlib.h>
#include <gpdef.h>

#define LCD_WIDTH 320
#define LCD_HEIGHT 240

extern char *	gp_strcpy(char *dst, const char *src);
extern char *	gp_strncpy(char *dst, const char *src, size_t count);
extern char *	gp_strcat(char *dst, const char *src);
extern char *	gp_strdup(const char *str);
extern int		gp_strcasecmp(const char *dst, const char *src);
extern int		gp_strncasecmp(const char *dst, const char *src, size_t count);
extern void		gp_sprintf(char *str, const char *fmt, ...);

extern void *	gp_memcpy(void *dst, const void *src, size_t count);
extern void *	gp_memset(void *dst, int val, size_t count);
extern void *	gp_malloc(size_t size);
extern void *	gp_calloc(size_t nitems, size_t size);
extern void 	gp_free(void *block);

extern void		gp_setCpuSpeed(int freq);
extern int		gp_printf(const char *fmt, ...);
extern void		gp_delay(unsigned long msecs);
extern void		gp_exit(int code);

extern void GPDEBUG(const char *fmt, ...);
extern void LP(const char *fmt, ...);
extern void NP(const char *fmt, ...);
extern void SP(const char *fmt, ...);
extern void BP(const char *fmt, ...);

#endif
