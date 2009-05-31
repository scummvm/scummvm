/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

/* libmspack -- a library for working with Microsoft compression formats.
 * (C) 2003-2004 Stuart Caie <kyzer@4u.net>
 *
 * This source code is adopted and striped for Residual project.
 *
 * libmspack is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License (LGPL) version 2.1
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef LIB_MSPACK_H
#define LIB_MSPACK_H

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct mspack_system {
  struct mspack_file * (*open)(struct mspack_system *self,
			       const char *filename,
			       int mode);
  void (*close)(struct mspack_file *file);
  int (*read)(struct mspack_file *file,
	      void *buffer,
	      int bytes);
  int (*write)(struct mspack_file *file,
	       void *buffer,
	       int bytes);
  int (*seek)(struct mspack_file *file,
	      off_t offset,
	      int mode);
  off_t (*tell)(struct mspack_file *file);
  void (*message)(struct mspack_file *file,
		  char *format,
		  ...);
  void * (*alloc)(struct mspack_system *self,
		  size_t bytes);
  void (*free)(void *ptr);
  void (*copy)(void *src,
	       void *dest,
	       size_t bytes);
  void *null_ptr;
};

#define MSPACK_SYS_OPEN_READ   (0)
#define MSPACK_SYS_OPEN_WRITE  (1)
#define MSPACK_SYS_OPEN_UPDATE (2)
#define MSPACK_SYS_OPEN_APPEND (3)

#define MSPACK_SYS_SEEK_START  (0)
#define MSPACK_SYS_SEEK_CUR    (1)
#define MSPACK_SYS_SEEK_END    (2)

struct mspack_file {
  int dummy;
};

#define MSPACK_ERR_OK          (0)
#define MSPACK_ERR_ARGS        (1)
#define MSPACK_ERR_OPEN        (2)
#define MSPACK_ERR_READ        (3)
#define MSPACK_ERR_WRITE       (4)
#define MSPACK_ERR_SEEK        (5)
#define MSPACK_ERR_NOMEMORY    (6)
#define MSPACK_ERR_SIGNATURE   (7)
#define MSPACK_ERR_DATAFORMAT  (8)
#define MSPACK_ERR_CHECKSUM    (9)
#define MSPACK_ERR_CRUNCH      (10)
#define MSPACK_ERR_DECRUNCH    (11)

extern struct mscab_decompressor *
  mspack_create_cab_decompressor(struct mspack_system *sys);

extern void mspack_destroy_cab_decompressor(struct mscab_decompressor *self);

struct mscabd_cabinet {
  struct mscabd_cabinet *next;
  char *filename;
  off_t base_offset;
  unsigned int length;
  struct mscabd_cabinet *prevcab;
  struct mscabd_cabinet *nextcab;
  char *prevname;
  char *nextname;
  char *previnfo;
  char *nextinfo;
  struct mscabd_file *files;
  struct mscabd_folder *folders;
  unsigned short set_id;
  unsigned short set_index;
  unsigned short header_resv;
  int flags;
};

#define MSCAB_HDR_RESV_OFFSET (0x28)
#define MSCAB_HDR_PREVCAB (0x01)
#define MSCAB_HDR_NEXTCAB (0x02)
#define MSCAB_HDR_RESV    (0x04)

struct mscabd_folder {
  struct mscabd_folder *next;
  int comp_type;
  unsigned int num_blocks;
};

#define MSCABD_COMP_METHOD(comp_type) ((comp_type) & 0x0F)
#define MSCABD_COMP_LEVEL(comp_type) (((comp_type) >> 8) & 0x1F)
#define MSCAB_COMP_MSZIP      (1)

struct mscabd_file {
  struct mscabd_file *next;
  char *filename;
  unsigned int length;
  int attribs;
  char time_h;
  char time_m;
  char time_s;
  char date_d;
  char date_m;
  int date_y;
  struct mscabd_folder *folder;
  unsigned int offset;
};

#define MSCAB_ATTRIB_RDONLY   (0x01)
#define MSCAB_ATTRIB_HIDDEN   (0x02)
#define MSCAB_ATTRIB_SYSTEM   (0x04)
#define MSCAB_ATTRIB_ARCH     (0x20)
#define MSCAB_ATTRIB_EXEC     (0x40)
#define MSCAB_ATTRIB_UTF_NAME (0x80)

#define MSCABD_PARAM_SEARCHBUF (0)
#define MSCABD_PARAM_FIXMSZIP  (1)
#define MSCABD_PARAM_DECOMPBUF (2)

struct mscab_decompressor {
  struct mscabd_cabinet * (*open) (struct mscab_decompressor *self,
				   char *filename);
  void (*close)(struct mscab_decompressor *self,
		struct mscabd_cabinet *cab);
  int (*extract)(struct mscab_decompressor *self,
		 struct mscabd_file *file,
		 char *filename);
  int (*last_error)(struct mscab_decompressor *self);
};

#endif
