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

/* This file is part of libmspack.
 * (C) 2003-2004 Stuart Caie.
 *
 * This source code is adopted and striped for Residual project.
 *
 * libmspack is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License (LGPL) version 2.1
 *
 * For further details, see the file COPYING.LIB distributed with libmspack
 */

#include "tools/patchex/cab.h"

static struct mscabd_cabinet * cabd_open(
  struct mscab_decompressor *base, char *filename);
static void cabd_close(
  struct mscab_decompressor *base, struct mscabd_cabinet *origcab);
static int cabd_read_headers(
  struct mspack_system *sys, struct mspack_file *fh,
  struct mscabd_cabinet_p *cab, off_t offset, int quiet);
static char *cabd_read_string(
  struct mspack_system *sys, struct mspack_file *fh,
  struct mscabd_cabinet_p *cab, int *error);

static int cabd_extract(
  struct mscab_decompressor *base, struct mscabd_file *file, char *filename);
static int cabd_init_decomp(
  struct mscab_decompressor_p *handle, unsigned int ct);
static void cabd_free_decomp(
  struct mscab_decompressor_p *handle);
static int cabd_sys_read(
  struct mspack_file *file, void *buffer, int bytes);
static int cabd_sys_write(
  struct mspack_file *file, void *buffer, int bytes);
static int cabd_sys_read_block(
  struct mspack_system *sys, struct mscabd_decompress_state *d, int *out,
  int ignore_cksum);
static unsigned int cabd_checksum(
  unsigned char *data, unsigned int bytes, unsigned int cksum);

static int cabd_error(
  struct mscab_decompressor *base);

#define __egi32(a,n) (  ((((unsigned char *) a)[n+3]) << 24) | \
			((((unsigned char *) a)[n+2]) << 16) | \
			((((unsigned char *) a)[n+1]) <<  8) | \
			((((unsigned char *) a)[n+0])))

#define EndGetI32(a) __egi32(a,0)
#define EndGetI16(a) ((((a)[1])<<8)|((a)[0]))

struct mscab_decompressor *
  mspack_create_cab_decompressor(struct mspack_system *sys)
{
  struct mscab_decompressor_p *handle = NULL;

  if ((handle = (mscab_decompressor_p *)sys->alloc(sys, sizeof(struct mscab_decompressor_p)))) {
    handle->base.open       = &cabd_open;
    handle->base.close      = &cabd_close;
    handle->base.extract    = &cabd_extract;
    handle->base.last_error = &cabd_error;
    handle->system          = sys;
    handle->d               = NULL;
    handle->error           = MSPACK_ERR_OK;

    handle->param[MSCABD_PARAM_SEARCHBUF] = 32768;
    handle->param[MSCABD_PARAM_FIXMSZIP]  = 0;
    handle->param[MSCABD_PARAM_DECOMPBUF] = 4096;
  }
  return (struct mscab_decompressor *) handle;
}

void mspack_destroy_cab_decompressor(struct mscab_decompressor *base) {
  struct mscab_decompressor_p *handle = (struct mscab_decompressor_p *) base;
  if (handle) {
    struct mspack_system *sys = handle->system;
    cabd_free_decomp(handle);
    if (handle->d) {
      if (handle->d->infh) sys->close(handle->d->infh);
      sys->free(handle->d);
    }
    sys->free(handle);
  }
}

static struct mscabd_cabinet *cabd_open(struct mscab_decompressor *base,
					char *filename)
{
  struct mscab_decompressor_p *handle = (struct mscab_decompressor_p *) base;
  struct mscabd_cabinet_p *cab = NULL;
  struct mspack_system *sys;
  struct mspack_file *fh;
  int error;

  if (!base) return NULL;
  sys = handle->system;

  if ((fh = sys->open(sys, filename, MSPACK_SYS_OPEN_READ))) {
    if ((cab = (mscabd_cabinet_p *)sys->alloc(sys, sizeof(struct mscabd_cabinet_p)))) {
      cab->base.filename = filename;
      error = cabd_read_headers(sys, fh, cab, (off_t) 0, 0);
      if (error) {
	cabd_close(base, (struct mscabd_cabinet *) cab);
	cab = NULL;
      }
      handle->error = error;
    }
    else {
      handle->error = MSPACK_ERR_NOMEMORY;
    }
    sys->close(fh);
  }
  else {
    handle->error = MSPACK_ERR_OPEN;
  }
  return (struct mscabd_cabinet *) cab;
}

static void cabd_close(struct mscab_decompressor *base,
		       struct mscabd_cabinet *origcab)
{
  struct mscab_decompressor_p *handle = (struct mscab_decompressor_p *) base;
  struct mscabd_folder_data *dat, *ndat;
  struct mscabd_cabinet *cab, *ncab;
  struct mscabd_folder *fol, *nfol;
  struct mscabd_file *fi, *nfi;
  struct mspack_system *sys;

  if (!base) return;
  sys = handle->system;

  handle->error = MSPACK_ERR_OK;

  while (origcab) {
    /* free files */
    for (fi = origcab->files; fi; fi = nfi) {
      nfi = fi->next;
      sys->free(fi->filename);
      sys->free(fi);
    }

    for (fol = origcab->folders; fol; fol = nfol) {
      nfol = fol->next;

      if (handle->d && (handle->d->folder == (struct mscabd_folder_p *) fol)) {
	if (handle->d->infh) sys->close(handle->d->infh);
	cabd_free_decomp(handle);
	sys->free(handle->d);
	handle->d = NULL;
      }

      for (dat = ((struct mscabd_folder_p *)fol)->data.next; dat; dat = ndat) {
	ndat = dat->next;
	sys->free(dat);
      }
      sys->free(fol);
    }

    for (cab = origcab; cab; cab = ncab) {
      ncab = cab->prevcab;
      sys->free(cab->prevname);
      sys->free(cab->nextname);
      sys->free(cab->previnfo);
      sys->free(cab->nextinfo);
      if (cab != origcab) sys->free(cab);
    }

    for (cab = origcab->nextcab; cab; cab = ncab) {
      ncab = cab->nextcab;
      sys->free(cab->prevname);
      sys->free(cab->nextname);
      sys->free(cab->previnfo);
      sys->free(cab->nextinfo);
      sys->free(cab);
    }

    cab = origcab->next;
    sys->free(origcab);

    origcab = cab;
  }
}

static int cabd_read_headers(struct mspack_system *sys,
			     struct mspack_file *fh,
			     struct mscabd_cabinet_p *cab,
			     off_t offset, int quiet)
{
  int num_folders, num_files, folder_resv, i, x;
  struct mscabd_folder_p *fol, *linkfol = NULL;
  struct mscabd_file *file, *linkfile = NULL;
  unsigned char buf[64];

  cab->base.next     = NULL;
  cab->base.files    = NULL;
  cab->base.folders  = NULL;
  cab->base.prevcab  = cab->base.nextcab  = NULL;
  cab->base.prevname = cab->base.nextname = NULL;
  cab->base.previnfo = cab->base.nextinfo = NULL;

  cab->base.base_offset = offset;

  if (sys->seek(fh, offset, MSPACK_SYS_SEEK_START)) {
    return MSPACK_ERR_SEEK;
  }

  if (sys->read(fh, &buf[0], cfhead_SIZEOF) != cfhead_SIZEOF) {
    return MSPACK_ERR_READ;
  }

  if (EndGetI32(&buf[cfhead_Signature]) != 0x4643534D) {
    return MSPACK_ERR_SIGNATURE;
  }

  cab->base.length    = EndGetI32(&buf[cfhead_CabinetSize]);
  cab->base.set_id    = EndGetI16(&buf[cfhead_SetID]);
  cab->base.set_index = EndGetI16(&buf[cfhead_CabinetIndex]);

  num_folders = EndGetI16(&buf[cfhead_NumFolders]);
  if (num_folders == 0) {
    if (!quiet) sys->message(fh, "no folders in cabinet.");
    return MSPACK_ERR_DATAFORMAT;
  }

  num_files = EndGetI16(&buf[cfhead_NumFiles]);
  if (num_files == 0) {
    if (!quiet) sys->message(fh, "no files in cabinet.");
    return MSPACK_ERR_DATAFORMAT;
  }

  if ((buf[cfhead_MajorVersion] != 1) && (buf[cfhead_MinorVersion] != 3)) {
    if (!quiet) sys->message(fh, "WARNING; cabinet version is not 1.3");
  }

  cab->base.flags = EndGetI16(&buf[cfhead_Flags]);
  if (cab->base.flags & cfheadRESERVE_PRESENT) {
    if (sys->read(fh, &buf[0], cfheadext_SIZEOF) != cfheadext_SIZEOF) {
      return MSPACK_ERR_READ;
    }
    cab->base.header_resv = EndGetI16(&buf[cfheadext_HeaderReserved]);
    folder_resv           = buf[cfheadext_FolderReserved];
    cab->block_resv       = buf[cfheadext_DataReserved];

    if (cab->base.header_resv > 60000) {
      if (!quiet) sys->message(fh, "WARNING; reserved header > 60000.");
    }

    if (cab->base.header_resv) {
      if (sys->seek(fh, (off_t) cab->base.header_resv, MSPACK_SYS_SEEK_CUR)) {
	return MSPACK_ERR_SEEK;
      }
    }
  }
  else {
    cab->base.header_resv = 0;
    folder_resv           = 0; 
    cab->block_resv       = 0;
  }

  if (cab->base.flags & cfheadPREV_CABINET) {
    cab->base.prevname = cabd_read_string(sys, fh, cab, &x); if (x) return x;
    cab->base.previnfo = cabd_read_string(sys, fh, cab, &x); if (x) return x;
  }

  if (cab->base.flags & cfheadNEXT_CABINET) {
    cab->base.nextname = cabd_read_string(sys, fh, cab, &x); if (x) return x;
    cab->base.nextinfo = cabd_read_string(sys, fh, cab, &x); if (x) return x;
  }

  for (i = 0; i < num_folders; i++) {
    if (sys->read(fh, &buf[0], cffold_SIZEOF) != cffold_SIZEOF) {
      return MSPACK_ERR_READ;
    }
    if (folder_resv) {
      if (sys->seek(fh, (off_t) folder_resv, MSPACK_SYS_SEEK_CUR)) {
	return MSPACK_ERR_SEEK;
      }
    }

    if (!(fol = (struct mscabd_folder_p *)sys->alloc(sys, sizeof(struct mscabd_folder_p)))) {
      return MSPACK_ERR_NOMEMORY;
    }
    fol->base.next       = NULL;
    fol->base.comp_type  = EndGetI16(&buf[cffold_CompType]);
    fol->base.num_blocks = EndGetI16(&buf[cffold_NumBlocks]);
    fol->data.next       = NULL;
    fol->data.cab        = (struct mscabd_cabinet_p *) cab;
    fol->data.offset     = offset + (off_t)
      ( (unsigned int) EndGetI32(&buf[cffold_DataOffset]) );
    fol->merge_prev      = NULL;
    fol->merge_next      = NULL;

    if (!linkfol) cab->base.folders = (struct mscabd_folder *) fol;
    else linkfol->base.next = (struct mscabd_folder *) fol;
    linkfol = fol;
  }

  for (i = 0; i < num_files; i++) {
    if (sys->read(fh, &buf[0], cffile_SIZEOF) != cffile_SIZEOF) {
      return MSPACK_ERR_READ;
    }

    if (!(file = (struct mscabd_file *)sys->alloc(sys, sizeof(struct mscabd_file)))) {
      return MSPACK_ERR_NOMEMORY;
    }

    file->next     = NULL;
    file->length   = EndGetI32(&buf[cffile_UncompressedSize]);
    file->attribs  = EndGetI16(&buf[cffile_Attribs]);
    file->offset   = EndGetI32(&buf[cffile_FolderOffset]);

    x = EndGetI16(&buf[cffile_FolderIndex]);
    if (x < cffileCONTINUED_FROM_PREV) {
      struct mscabd_folder *ifol = cab->base.folders; 
      while (x--) if (ifol) ifol = ifol->next;
      file->folder = ifol;

      if (!ifol) {
	sys->free(file);
	return MSPACK_ERR_DATAFORMAT;
      }
    }
    else {
      if ((x == cffileCONTINUED_TO_NEXT) ||
	  (x == cffileCONTINUED_PREV_AND_NEXT))
      {
	struct mscabd_folder *ifol = cab->base.folders;
	while (ifol->next) ifol = ifol->next;
	file->folder = ifol;

	fol = (struct mscabd_folder_p *) ifol;
	if (!fol->merge_next) fol->merge_next = file;
      }

      if ((x == cffileCONTINUED_FROM_PREV) ||
	  (x == cffileCONTINUED_PREV_AND_NEXT))
      {
	file->folder = cab->base.folders;

	fol = (struct mscabd_folder_p *) file->folder;
	if (!fol->merge_prev) fol->merge_prev = file;
      }
    }

    x = EndGetI16(&buf[cffile_Time]);
    file->time_h = x >> 11;
    file->time_m = (x >> 5) & 0x3F;
    file->time_s = (x << 1) & 0x3E;

    x = EndGetI16(&buf[cffile_Date]);
    file->date_d = x & 0x1F;
    file->date_m = (x >> 5) & 0xF;
    file->date_y = (x >> 9) + 1980;

    file->filename = cabd_read_string(sys, fh, cab, &x);
    if (x) { 
      sys->free(file);
      return x;
    }

    if (!linkfile) cab->base.files = file;
    else linkfile->next = file;
    linkfile = file;
  }

  return MSPACK_ERR_OK;
}

static char *cabd_read_string(struct mspack_system *sys,
			      struct mspack_file *fh,
			      struct mscabd_cabinet_p *, int *error)
{
  off_t base = sys->tell(fh);
  char buf[256], *str;
  unsigned int len, i, ok;

  len = sys->read(fh, &buf[0], 256);

  for (i = 0, ok = 0; i < len; i++) if (!buf[i]) { ok = 1; break; }
  if (!ok) {
    *error = MSPACK_ERR_DATAFORMAT;
    return NULL;
  }

  len = i + 1;

  if (sys->seek(fh, base + (off_t)len, MSPACK_SYS_SEEK_START)) {
    *error = MSPACK_ERR_SEEK;
    return NULL;
  }

  if (!(str = (char *)sys->alloc(sys, len))) {
    *error = MSPACK_ERR_NOMEMORY;
    return NULL;
  }

  sys->copy(&buf[0], str, len);
  *error = MSPACK_ERR_OK;
  return str;
}
    
static int cabd_extract(struct mscab_decompressor *base,
			 struct mscabd_file *file, char *filename)
{
  struct mscab_decompressor_p *handle = (struct mscab_decompressor_p *) base;
  struct mscabd_folder_p *fol;
  struct mspack_system *sys;
  struct mspack_file *fh;

  if (!handle) return MSPACK_ERR_ARGS;
  if (!file) return handle->error = MSPACK_ERR_ARGS;

  sys = handle->system;
  fol = (struct mscabd_folder_p *) file->folder;

  /* check if file can be extracted */
  if ((!fol) || (fol->merge_prev) ||
      (((file->offset + file->length) / CAB_BLOCKMAX) > fol->base.num_blocks))
  {
    sys->message(NULL, "ERROR; file \"%s\" cannot be extracted, "
		 "cabinet set is incomplete.", file->filename);
    return handle->error = MSPACK_ERR_DATAFORMAT;
  }

  if (!handle->d) {
    handle->d = (mscabd_decompress_state *)sys->alloc(sys, sizeof(struct mscabd_decompress_state));
    if (!handle->d) return handle->error = MSPACK_ERR_NOMEMORY;
    handle->d->folder     = NULL;
    handle->d->data       = NULL;
    handle->d->sys        = *sys;
    handle->d->sys.read   = &cabd_sys_read;
    handle->d->sys.write  = &cabd_sys_write;
    handle->d->state      = NULL;
    handle->d->infh       = NULL;
    handle->d->incab      = NULL;
  }

  if ((handle->d->folder != fol) || (handle->d->offset > file->offset)) {
    if (!handle->d->infh || (fol->data.cab != handle->d->incab)) {
      if (handle->d->infh) sys->close(handle->d->infh);
      handle->d->incab = fol->data.cab;
      handle->d->infh = sys->open(sys, fol->data.cab->base.filename,
				MSPACK_SYS_OPEN_READ);
      if (!handle->d->infh) return handle->error = MSPACK_ERR_OPEN;
    }
    if (sys->seek(handle->d->infh, fol->data.offset, MSPACK_SYS_SEEK_START)) {
      return handle->error = MSPACK_ERR_SEEK;
    }

    if (cabd_init_decomp(handle, (unsigned int) fol->base.comp_type)) {
      return handle->error;
    }

    handle->d->folder = fol;
    handle->d->data   = &fol->data;
    handle->d->offset = 0;
    handle->d->block  = 0;
    handle->d->i_ptr = handle->d->i_end = &handle->d->input[0];
  }

  if (!(fh = sys->open(sys, filename, MSPACK_SYS_OPEN_WRITE))) {
    return handle->error = MSPACK_ERR_OPEN;
  }

  handle->error = MSPACK_ERR_OK;

  if (file->length) {
    off_t bytes;
    int error;
    handle->d->outfh = NULL;
    if ((bytes = file->offset - handle->d->offset)) {
      error = handle->d->decompress(handle->d->state, bytes);
      if (error != MSPACK_ERR_READ) handle->error = error;
    }

    if (!handle->error) {
      handle->d->outfh = fh;
      error = handle->d->decompress(handle->d->state, (off_t) file->length);
      if (error != MSPACK_ERR_READ) handle->error = error;
    }
  }

  sys->close(fh);
  handle->d->outfh = NULL;

  return handle->error;
}

static int cabd_init_decomp(struct mscab_decompressor_p *handle, unsigned int ct)
{
  struct mspack_file *fh = (struct mspack_file *) handle;

  if (!handle || !handle->d) {
    return handle->error = MSPACK_ERR_ARGS;
  }

  cabd_free_decomp(handle);

  handle->d->comp_type = ct;

  switch (ct & cffoldCOMPTYPE_MASK) {
  case cffoldCOMPTYPE_MSZIP:
    handle->d->decompress = (int (*)(void *, off_t)) &mszipd_decompress;
    handle->d->state = mszipd_init(&handle->d->sys, fh, fh,
				 handle->param[MSCABD_PARAM_DECOMPBUF],
				 handle->param[MSCABD_PARAM_FIXMSZIP]);
    break;
  default:
    return handle->error = MSPACK_ERR_DATAFORMAT;
  }
  return handle->error = (handle->d->state) ? MSPACK_ERR_OK : MSPACK_ERR_NOMEMORY;
}

static void cabd_free_decomp(struct mscab_decompressor_p *handle) {
  if (!handle || !handle->d || !handle->d->folder || !handle->d->state) return;

  switch (handle->d->comp_type & cffoldCOMPTYPE_MASK) {
  case cffoldCOMPTYPE_MSZIP:   mszipd_free((mszipd_stream *)handle->d->state);  break;
  }
  handle->d->decompress = NULL;
  handle->d->state      = NULL;
}

static int cabd_sys_read(struct mspack_file *file, void *buffer, int bytes) {
  struct mscab_decompressor_p *handle = (struct mscab_decompressor_p *) file;
  unsigned char *buf = (unsigned char *) buffer;
  struct mspack_system *sys = handle->system;
  int avail, todo, outlen = 0, ignore_cksum;

  ignore_cksum = handle->param[MSCABD_PARAM_FIXMSZIP] &&
    ((handle->d->comp_type & cffoldCOMPTYPE_MASK) == cffoldCOMPTYPE_MSZIP);

  todo = bytes;
  while (todo > 0) {
    avail = handle->d->i_end - handle->d->i_ptr;

    if (avail) {
      if (avail > todo) avail = todo;
      sys->copy(handle->d->i_ptr, buf, (size_t) avail);
      handle->d->i_ptr += avail;
      buf  += avail;
      todo -= avail;
    }
    else {
      if (handle->d->block++ >= handle->d->folder->base.num_blocks) {
	handle->error = MSPACK_ERR_DATAFORMAT;
	break;
      }

      handle->error = cabd_sys_read_block(sys, handle->d, &outlen, ignore_cksum);
      if (handle->error) return -1;

      if (handle->d->block >= handle->d->folder->base.num_blocks) {
      } else {
	if (outlen != CAB_BLOCKMAX) {
	  handle->system->message(handle->d->infh,
				"WARNING; non-maximal data block");
	}
      }
    }
  }
  return bytes - todo;
}

static int cabd_sys_write(struct mspack_file *file, void *buffer, int bytes) {
  struct mscab_decompressor_p *handle = (struct mscab_decompressor_p *) file;
  handle->d->offset += bytes;
  if (handle->d->outfh) {
    return handle->system->write(handle->d->outfh, buffer, bytes);
  }
  return bytes;
}

static int cabd_sys_read_block(struct mspack_system *sys,
			       struct mscabd_decompress_state *d,
			       int *out, int ignore_cksum)
{
  unsigned char hdr[cfdata_SIZEOF];
  unsigned int cksum;
  int len;

  d->i_ptr = d->i_end = &d->input[0];

  do {
    if (sys->read(d->infh, &hdr[0], cfdata_SIZEOF) != cfdata_SIZEOF) {
      return MSPACK_ERR_READ;
    }

    if (d->data->cab->block_resv &&
	sys->seek(d->infh, (off_t) d->data->cab->block_resv,
		  MSPACK_SYS_SEEK_CUR))
    {
      return MSPACK_ERR_SEEK;
    }

    len = EndGetI16(&hdr[cfdata_CompressedSize]);
    if (((d->i_end - d->i_ptr) + len) > CAB_INPUTMAX) {
      return MSPACK_ERR_DATAFORMAT;
    }

    if (EndGetI16(&hdr[cfdata_UncompressedSize]) > CAB_BLOCKMAX) {
      return MSPACK_ERR_DATAFORMAT;
    }

    if (sys->read(d->infh, d->i_end, len) != len) {
      return MSPACK_ERR_READ;
    }

    if ((cksum = EndGetI32(&hdr[cfdata_CheckSum]))) {
      unsigned int sum2 = cabd_checksum(d->i_end, (unsigned int) len, 0);
      if (cabd_checksum(&hdr[4], 4, sum2) != cksum) {
	if (!ignore_cksum) return MSPACK_ERR_CHECKSUM;
	sys->message(d->infh, "WARNING; bad block checksum found");
      }
    }

    d->i_end += len;

    if ((*out = EndGetI16(&hdr[cfdata_UncompressedSize]))) {
      return MSPACK_ERR_OK;
    }

    sys->close(d->infh);
    d->infh = NULL;

    if (!(d->data = d->data->next)) {
      return MSPACK_ERR_DATAFORMAT;
    }

    d->incab = d->data->cab;
    if (!(d->infh = sys->open(sys, d->incab->base.filename,
			      MSPACK_SYS_OPEN_READ)))
    {
      return MSPACK_ERR_OPEN;
    }

    if (sys->seek(d->infh, d->data->offset, MSPACK_SYS_SEEK_START)) {
      return MSPACK_ERR_SEEK;
    }
  } while (1);

  return MSPACK_ERR_OK;
}

static unsigned int cabd_checksum(unsigned char *data, unsigned int bytes,
				  unsigned int cksum)
{
  unsigned int len, ul = 0;

  for (len = bytes >> 2; len--; data += 4) {
    cksum ^= ((data[0]) | (data[1]<<8) | (data[2]<<16) | (data[3]<<24));
  }

  switch (bytes & 3) {
  case 3: ul |= *data++ << 16;
  case 2: ul |= *data++ <<  8;
  case 1: ul |= *data;
  }
  cksum ^= ul;

  return cksum;
}

static int cabd_error(struct mscab_decompressor *base) {
  struct mscab_decompressor_p *handle = (struct mscab_decompressor_p *) base;
  return (handle) ? handle->error : MSPACK_ERR_ARGS;
}
