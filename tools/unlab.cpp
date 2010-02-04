/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

struct lab_header {
	uint32_t magic;
	uint32_t magic2;
	uint32_t num_entries;
	uint32_t string_table_size;
};

struct lab_entry {
	uint32_t fname_offset;
	uint32_t start;
	uint32_t size;
	uint32_t reserved;
};

uint16_t READ_LE_UINT16(const void *ptr) {
	const uint8_t *b = (const uint8_t *)ptr;
	return (b[1] << 8) + b[0];
}
uint32_t READ_LE_UINT32(const void *ptr) {
	const uint8_t *b = (const uint8_t *)ptr;
	return (b[3] << 24) + (b[2] << 16) + (b[1] << 8) + (b[0]);
}

int main(int argc, char **argv) {
	FILE *infile, *outfile;
	struct lab_header head;
	struct lab_entry *entries;
	char *str_table;
	uint32_t i;
	off_t offset;

	infile = fopen(argv[1], "rb");
	if (infile == 0) {
		printf("can't open source file: %s\n", argv[1]);
		exit(1);
	}

	fread(&head.magic, 1, 4, infile);
	fread(&head.magic2, 1, 4, infile);
	uint32_t num, s_size;
	fread(&num, 1, 4, infile);
	fread(&s_size, 1, 4, infile);
	head.num_entries = READ_LE_UINT32(&num);
	head.string_table_size = READ_LE_UINT32(&s_size);
	if (0 != memcmp(&head.magic, "LABN", 4)) {
		printf("There is no LABN header in source file\n");
		exit(1);
	}

	entries = (struct lab_entry *)malloc(head.num_entries * sizeof(struct lab_entry));
	fread(entries, 1, head.num_entries * sizeof(struct lab_entry), infile);

	str_table = (char *)malloc(head.string_table_size);
	fread(str_table, 1, head.string_table_size, infile);

	for (i = 0; i < head.num_entries; i++) {
		outfile = fopen(str_table + READ_LE_UINT32(&entries[i].fname_offset), "wb");
		offset = READ_LE_UINT32(&entries[i].start);
		char *buf = (char *)malloc(READ_LE_UINT32(&entries[i].size));
		fseek(infile, offset, SEEK_SET);
		fread(buf, 1, READ_LE_UINT32(&entries[i].size), infile);
		fwrite(buf, 1, READ_LE_UINT32(&entries[i].size), outfile);
		fclose(outfile);
		free(buf);
	}

	fclose(infile);
	free(str_table);
	free(entries);
	return 0;
}
