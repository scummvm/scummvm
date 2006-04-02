/* Residual - Virtual machine to run LucasArts' 3D adventure games
 * Copyright (C) 2003-2006 The ScummVM-Residual Team (www.scummvm.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

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

#include "util.h"

struct lab_header {
  uint32 magic;
  uint32 magic2;
  uint32 num_entries;
  uint32 string_table_size;
};

struct lab_entry {
  uint32 fname_offset;
  uint32 start;
  uint32 size;
  uint32 reserved;
};

uint16 READ_LE_UINT16(const void *ptr) {
	const byte *b = (const byte *)ptr;
	return (b[1] << 8) + b[0];
}
uint32 READ_LE_UINT32(const void *ptr) {
	const byte *b = (const byte *)ptr;
	return (b[3] << 24) + (b[2] << 16) + (b[1] << 8) + (b[0]);
}

int main(int argc, char **argv) {
  FILE *infile, *outfile;
  struct lab_header head;
  struct lab_entry *entries;
  char *str_table;
  uint i;
  off_t offset;

  infile = fopen(argv[1], "rb");
  if (infile == 0)
  {
    error("can't open source file: %s", argv[1]);
  }

  fread(&head.magic, 1, 4, infile);
  fread(&head.magic2, 1, 4, infile);
  head.num_entries = readUint32LE(infile);
  head.string_table_size = readUint32LE(infile);
  if (0 != memcmp(&head.magic, "LABN", 4))
  {
    error("There is no LABN header in source file");
  }

  entries = (struct lab_entry *)malloc(head.num_entries * sizeof(struct lab_entry));
  fread(entries, 1, head.num_entries * sizeof(struct lab_entry), infile);

  str_table = (char *) malloc(head.string_table_size);
  fread(str_table, 1, head.string_table_size, infile);

  for (i = 0; i < head.num_entries; i++) {
    outfile = fopen(str_table + READ_LE_UINT32(&entries[i].fname_offset), "wb");
    offset = READ_LE_UINT32(&entries[i].start);
    char *buf = (char *)malloc(READ_LE_UINT32(&entries[i].size));
    fseek(infile, offset, SEEK_SET);
    fread(buf, 1, READ_LE_UINT32(&entries[i].size), infile);
    fwrite(buf, 1, READ_LE_UINT32(&entries[i].size), outfile);
    fclose(outfile);
  }

  fclose(infile);
  return 0;
}
