// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

struct lab_header {
  long magic;
  long magic2;
  long num_entries;
  long string_table_size;
};

struct lab_entry {
  long fname_offset;
  long start;
  long size;
  long reserved;
};

int main(int argc, char **argv) {
  FILE *infile, *outfile;
  struct lab_header head;
  struct lab_entry *entries;
  char *str_table;
  int i;
  off_t offset;

  infile = fopen(argv[1], "rb");
  if (infile == 0)
  {
    printf("can't open source file: %s\n", argv[1]);
    exit(1);
  }

  fread(&head, 1, sizeof(head), infile);
  if (head.magic != 'NBAL')
  {
    printf("its not LABN header in source file");
    exit(1);
  }

  entries = (struct lab_entry *)malloc(head.num_entries * sizeof(struct lab_entry));
  fread(entries, 1, head.num_entries * sizeof(struct lab_entry), infile);

  str_table = (char *) malloc(head.string_table_size);
  fread(str_table, 1, head.string_table_size, infile);

  for (i = 0; i < head.num_entries; i++) {
    outfile = fopen(str_table + entries[i].fname_offset, "wb");
    offset = entries[i].start;
    char *buf = (char *)malloc(entries[i].size);
    fseek(infile, offset, SEEK_SET);
    fread(buf, 1, entries[i].size, infile);
    fwrite(buf, 1, entries[i].size, outfile);
    fclose(outfile);
  }

  fclose(infile);
  return 0;
}
