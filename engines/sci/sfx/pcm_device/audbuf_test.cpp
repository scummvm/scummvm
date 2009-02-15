/***************************************************************************
 audbuf_test.c  Copyright (C) 2002 Christoph Reichenbach


 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public Licence as
 published by the Free Software Foundaton; either version 2 of the
 Licence, or (at your option) any later version.

 It is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 merchantibility or fitness for a particular purpose. See the
 GNU General Public Licence for more details.

 You should have received a copy of the GNU General Public Licence
 along with this program; see the file COPYING. If not, write to
 the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 Boston, MA 02111-1307, USA.


 Please contact the maintainer for any program-related bug reports or
 inquiries.

 Current Maintainer:

    Christoph Reichenbach (CR) <jameson@linuxgames.com>

***************************************************************************/

#include "audiobuf.h"
#if 0
sfx_audio_buf_t buf;

#define MIN_FRAMESIZE 1
#define MAX_FRAMESIZE 8


void
tester_write(unsigned char *data, int datalen, int framesize, int gran) {
	int i;

	for (i = 0; i < datalen; i += gran) {
		int size = (i + gran < datalen) ? gran : datalen - i;

		sfx_audbuf_write(&buf, data + (i * framesize), framesize, size);
	}
}


void
tester_read(unsigned char *data, int datalen, int framesize, int gran) {
	unsigned char *readdata = malloc(datalen * framesize);
	int i;

	for (i = 0; i < datalen; i += gran) {
		int size = (i + gran < datalen) ? gran : datalen - i;
		int j;

		sfx_audbuf_read(&buf, readdata + (i * framesize), framesize, size);
		for (j = 0; j < gran * framesize; j++) {
			int offset = i * framesize + j;

			if (data[i] != readdata[i]) {
				fprintf(stderr, "[ERROR] Mismatch at offset %08x (sample #%d): Expected %02x, got %02x\n",
				        offset, i,  readdata[i], data[i]);
			}
		}
	}

	free(readdata);
}


void
test1(unsigned char *data, int len)
/* Test the 'regular' case */
{
	int framesize;
	int stepsize;

	fprintf(stderr, "[Test-1] Commenced; len=%d.\n", len);

	for (framesize = MAX_FRAMESIZE; framesize >= MIN_FRAMESIZE; framesize >>= 1) {
		fprintf(stderr, "[Test-1] Writing frame size %d\n", framesize);
		for (stepsize = 1; stepsize <= len; stepsize++)
			tester_write(data, len / framesize, framesize, stepsize);
	}

	for (framesize = MAX_FRAMESIZE; framesize >= MIN_FRAMESIZE; framesize >>= 1) {
		fprintf(stderr, "[Test-1] Reading frame size %d\n", framesize);
		for (stepsize = len; stepsize >= 1; stepsize--)
			tester_read(data, len / framesize, framesize, stepsize);
	}

	fprintf(stderr, "[Test-1] Completed.\n");
}

#define TEST2_COUNT 10
#define TEST2_LEN 3

void
test2(unsigned char *data, int framesize)
/* Test whether buffer underrun repeats are handled correctly */
{
	int i;
	unsigned char *src;

	fprintf(stderr, "[Test-2] Commenced; framesize=%d.\n", framesize);

	sfx_audbuf_write(&buf, data, framesize, 1);
	src = malloc(framesize * TEST2_LEN);

	for (i = 0; i < TEST2_COUNT + 1; i++) {
		int inst;
		sfx_audbuf_read(&buf, src, framesize, TEST2_LEN);

		for (inst = 0; inst < TEST2_LEN; inst++) {
			int offset = inst * framesize;
			int j;

			for (j = 0; j < framesize; j++)
				if (src[j + offset] != data[j]) {
					fprintf(stderr, "[ERROR] At copy sample %d, frame %d, offset %d: Expected %02x, got %02x\n",
					        i, inst, j, data[j], src[j+offset]);
				}
		}
		memset(src, 0xbf, framesize * TEST2_LEN);
	}

	free(src);
	fprintf(stderr, "[Test-1] Completed.\n");
}


#define CHUNKS_NR 4

#define CHUNK_LEN_0 8
#define CHUNK_LEN_1 20
#define CHUNK_LEN_2 16
#define CHUNK_LEN_3 40

unsigned char test_data_0[CHUNK_LEN_0] = { 0x01, 0x02, 0x03, 0x04,  0x05, 0x06, 0x07, 0x08 };
unsigned char test_data_1[CHUNK_LEN_1] = { 0xff, 0xff, 0xff, 0xff,  0xff, 0xff, 0xff, 0xff,
        0x00, 0xff, 0x00, 0xff,  0x00, 0xff, 0x00, 0xff,
        0xff, 0x00, 0xff, 0x00
                                         };
unsigned char test_data_2[CHUNK_LEN_2] = { 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00
                                         };
unsigned char test_data_3[CHUNK_LEN_3] = { 0x01, 0x02, 0x03, 0x04,  0x05, 0x06, 0x07, 0x08,
        0x11, 0x12, 0x13, 0x14,  0x15, 0x16, 0x17, 0x18,
        0x21, 0x22, 0x23, 0x24,  0x25, 0x26, 0x27, 0x28,
        0x41, 0x42, 0x43, 0x44,  0x45, 0x46, 0x47, 0x48,
        0x8f, 0x8e, 0x8d, 0x8c,  0x8b, 0x8a, 0x89, 0x88
                                         };

struct {
	int len;
	unsigned char *data;
} test_chunks[CHUNKS_NR] = {
	{ CHUNK_LEN_0, test_data_0 },
	{ CHUNK_LEN_1, test_data_1 },
	{ CHUNK_LEN_2, test_data_2 },
	{ CHUNK_LEN_3, test_data_3 }
};

int
main(int argc, char **argv) {
	int i;

	sfx_audbuf_init(&buf);
	for (i = 0; i < CHUNKS_NR; i++) {
		int k;

		/*		for (k = MAX_FRAMESIZE; k >= MIN_FRAMESIZE; k >>= 1)
				test2(test_chunks[i].data, k);*/

		test1(test_chunks[i].data, test_chunks[i].len);
	}
	sfx_audbuf_exit(&buf);

	return 0;
}
#else
int main() {}
#endif
