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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

// HACK to allow building with the SDL backend on MinGW
// see bug #1800764 "TOOLS: MinGW tools building broken"
#ifdef main
#undef main
#endif // main

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "md5.h"

static void print_hex(FILE * f, const uint8 * data, size_t len) {
	for (size_t i = 0; i < len; ++i) {
		fprintf(f, "%02x", data[i]);
	}
}

static void extract(FILE * fout, FILE *fin, size_t pos, size_t size, const char *what) {
	char buf[0x10000];
	assert(size < sizeof(buf));

	if (fseek(fin, pos, SEEK_SET) != 0) {
		perror(what);
		exit(1);
	}

	if (fread(buf, size, 1, fin) != 1) {
		perror(what);
		exit(1);
	}

	if (fwrite(buf, size, 1, fout) != 1) {
		perror(what);
		exit(1);
	}
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "usage: %s: Teenagnt.exe (unpacked one)\n", argv[0]);
		exit(1);
	}
	const char * fname = argv[1];

	uint8 digest[16];
	if (!md5_file(fname, digest, 0)) {
		fprintf(stderr, "cannot calculate md5 for %s", fname);
		exit(1);
	}

	const uint8 ethalon[16] = {
		0x51, 0xb6, 0xd6, 0x47, 0x21, 0xf7, 0xc4, 0xb4,
		0x98, 0xbf, 0xc0, 0xf3, 0x23, 0x01, 0x3e, 0x36,
	};

	if (memcmp(digest, ethalon, 16) != 0) {
		fprintf(stderr, "cannot extract data, your md5: ");
		print_hex(stderr, digest, 16);
		fprintf(stderr, ", need md5: ");
		print_hex(stderr, ethalon, 16);
		fprintf(stderr, ", sorry\n");
		exit(1);
	}
	FILE *fin = fopen(fname, "rb");
	if (fin == NULL) {
		perror("opening input file");
		exit(1);
	}

	const char * dat_name = "teenagent.dat";
	FILE *fout = fopen(dat_name, "wb");
	if (fout == NULL) {
		perror("opening output file");
		exit(1);
	}
	//0x0200, 0xb5b0, 0x1c890
	extract(fout, fin, 0x00200, 0xb3b0, "extracting code segment");
	extract(fout, fin, 0x0b5b0, 0xe790, "extracting data segment");
	extract(fout, fin, 0x1c890, 0x8be2, "extracting second data segment");

	fclose(fin);
	fclose(fout);

	fprintf(stderr, "please run \"gzip -n %s\"\n", dat_name);

	return 0;
}
