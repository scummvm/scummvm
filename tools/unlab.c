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
#include <fcntl.h>
#include <stdlib.h>

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
  int infile, outfile;
  struct lab_header head;
  struct lab_entry *entries;
  char *str_table;
  int i;
  off_t offset;

  infile = open(argv[1], O_RDONLY);
  if (infile < 0)
    exit(1);

  read(infile, &head, sizeof(head));
  if (head.magic != 'NBAL')
    exit(1);

  entries = (struct lab_entry *)
    malloc(head.num_entries * sizeof(struct lab_entry));
  read(infile, entries, head.num_entries * sizeof(struct lab_entry));

  str_table = (char *) malloc(head.string_table_size);
  read(infile, str_table, head.string_table_size);

  for (i = 0; i < head.num_entries; i++) {
    outfile = open(str_table + entries[i].fname_offset,
		   O_CREAT | O_TRUNC | O_WRONLY, 0666);
    offset = entries[i].start;
    sendfile(outfile, infile, &offset, entries[i].size);
    close(outfile);
  }

  return 0;
}
