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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * Patches Myth C64 disk image
 * Written by David Kinder, based on code by Niclas Karlsson
 */

#define D64_SIZE (174848L)
unsigned char image[D64_SIZE];

void ungarble(unsigned char *block, signed long key) {
	unsigned char d;
	int i, j;

	d = (unsigned char)((key & 0x07) ^ 0xFF);
	if (d < 0xFF) {
		i = d;
		j = d + 1;
		while (j < 0x100)
			block[j++] ^= block[i];
	}

	i = 0xFF;
	j = d - 1;
	while (j >= 0)
		block[j--] ^= block[i--];

	for (i = 0; i < 128; i++) {
		d = block[i];
		block[i] = block[255 - i];
		block[255 - i] = d;
	}
}

void garble(unsigned char *block, int i1, int j1, int i2) {
	unsigned char d;
	int i, j;

	for (i = 0; i < 128; i++) {
		d = block[i];
		block[i] = block[255 - i];
		block[255 - i] = d;
	}

	i = i1;
	j = -1;
	while (j < j1)
		block[++j] ^= block[++i];

	i = i2;
	j = 0x100;
	while (j > i2 + 1)
		block[--j] ^= block[i];
}

/* This routine does the patching. */
void fixDiskImage(int code) {
	unsigned char *block = NULL;

	switch (code) {
	case 0:
		/* code[0x3056] = 0x60 */
		block = image + 0x1600;
		ungarble(block, 0x27L);
		block[0x56] = 0x60;
		garble(block, 0x07, 0xF7, 0xF8);
		break;
	case 1:
		/* code[0x3148] = 0x60 */
		/* code[0x314D] = 0x11 */
		block = image + 0x1700;
		ungarble(block, 0x28L);
		block[0x48] = 0x60;
		block[0x4D] = 0x11;
		garble(block, 0x00, 0xFE, 0xFF);
		break;
	}
}

void readDiskImage(const char *filename) {
	FILE *file = fopen(filename, "r+b");
	if (file == NULL)
		exit(1);
	fread(image, 1, D64_SIZE, file);
}

void writeDiskImage(const char *filename) {
	FILE *file = fopen(filename, "w+b");
	if (file == NULL)
		exit(1);
	fwrite(image, 1, D64_SIZE, file);
}

int main(int argc, char **argv) {
	if (argc == 4) {
		int code = -1;
		if (sscanf(argv[3], "%d", &code) == 1) {
			if ((code >= 0) && (code <= 1)) {
				readDiskImage(argv[1]);
				fixDiskImage(code);
				writeDiskImage(argv[2]);
				printf("Myth patched.\n");
				return 0;
			}
		}
	}

	printf("Removes password protection from Magnetic Scrolls' Myth.\n");
	printf("Use: myth input.d64 output.d64 code\n");
	printf("If code is 0, the password protection is completely removed.\n");
	printf("If code is 1, any user name and password will be accepted.\n");
	return 0;
}
