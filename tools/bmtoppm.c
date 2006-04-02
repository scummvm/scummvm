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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <assert.h>

#include <ppm.h>

int32_t read_LEint32(FILE *f) {
  unsigned char c[4];

  fread(c, 1, 4, f);
  return (c[0]) | (c[1] << 8) | (c[2] << 16) | (c[3] << 24);
}

void read_header(FILE *in, int *codec, int *num_images, int *format) {
  char magic[8];

  fread(magic, 1, 8, in);
  if (memcmp(magic, "BM  F\0\0\0", 8) != 0) {
    fprintf(stderr, "not a bitmap file\n");
    exit(1);
  }
  *codec = read_LEint32(in);

  read_LEint32(in);
  *num_images = read_LEint32(in);
  read_LEint32(in);		/* ignore x offset */
  read_LEint32(in);		/* ignore y offset */
  read_LEint32(in);
  *format = read_LEint32(in);
  fseek(in, 128, SEEK_SET);
}

void read_data_codec0(FILE *in, int width, int height, unsigned char *result) {
  fread(result, 1, width * height * 2, in);
}

void read_data_codec3(FILE *in, int size, unsigned char *result) {
  unsigned char *data;
  const unsigned char *data_ptr;
  int bitstr_val, bitstr_len;
  int offset, len;

  data = malloc(size);
  fread(data, 1, size, in);

  bitstr_val = data[0] | (data[1] << 8);
  bitstr_len = 16;
  data_ptr = data + 2;

#define GET_BIT ({ \
    int bit_result = bitstr_val & 1; \
    bitstr_val >>= 1; \
    bitstr_len--; \
    if (bitstr_len == 0) { \
      bitstr_val = data_ptr[0] | (data_ptr[1] << 8); \
      bitstr_len = 16; \
      data_ptr += 2; \
    } \
    bit_result; \
  })

  for (;;) {
    if (GET_BIT == 1)
      *result++ = *data_ptr++;
    else {
      if (GET_BIT == 0) {
	len = GET_BIT * 2;
	len += GET_BIT;
	len += 3;
	offset = *data_ptr - 0x100;
	data_ptr++;
      }
      else {
	offset = data_ptr[0] | ((data_ptr[1] & 0xf0) << 4);
	offset -= 0x1000;
	len = (data_ptr[1] & 0xf) + 3;
	data_ptr += 2;
	if (len == 3) {
	  len = *data_ptr++;
	  len++;
	  if (len == 1) {
	    free(data);
	    return;
	  }
	}
      }
      while (len > 0) {
	*result = result[offset];
	result++;
	len--;
      }
    }
  }
}

void write_img(pixel **img, const char *fname, int img_num,
	       int width, int height, int maxval) {
  const char *basename;
  char newname[1024];
  FILE *out;

  basename = strrchr(fname, '/');
  if (basename != NULL)
    basename++;
  else
    basename = fname;
  strcpy(newname, basename);
  if (strlen(newname) > 3 &&
      strcasecmp(newname + strlen(newname) - 3, ".bm") == 0)
    newname[strlen(newname) - 3] = '\0';
  else if (strlen(newname) > 4 &&
	   strcasecmp(newname + strlen(newname) - 4, ".zbm") == 0) {
    newname[strlen(newname) - 4] = '\0';
    strcat(newname, "_z");
  }
  sprintf(newname + strlen(newname), "_%d.ppm", img_num);

  out = fopen(newname, "wb");
  if (out == NULL) {
    perror(newname);
    exit(1);
  }

  ppm_writeppm(out, img, width, height, maxval, 0);
  fclose(out);
}

pixel **toimg_fmt1(const unsigned char *data, int width, int height) {
  pixel **result;
  int x, y;
  int r, g, b;

  result = ppm_allocarray(width, height);
  for (y = 0; y < height; y++)
    for (x = 0; x < width; x++) {
      r = data[1] >> 3;
      r = (r << 3) | (r >> 2);
      g = ((data[1] & 7) << 3) | (data[0] >> 5);
      g = (g << 2) | (g >> 4);
      b = data[0] & 0x1f;
      b = (b << 3) | (b >> 2);
      PPM_ASSIGN(result[y][x], r, g, b);
      data += 2;
    }
  return result;
}

pixel **toimg_fmt5(const unsigned char *data, int width, int height) {
  pixel **result;
  int x, y;
  unsigned short gray;

  result = ppm_allocarray(width, height);
  for (y = 0; y < height; y++)
    for (x = 0; x < width; x++) {
      /*      gray = data[0] | (data[1] << 8); */
      gray = data[1];
      PPM_ASSIGN(result[y][x], gray, gray, gray);
      data += 2;
    }
  return result;
}

void process_file(const char *fname) {
  FILE *in;
  int codec, num_images, format;
  int i;
  int width, height, size, maxval;
  unsigned char *data;
  pixel **img;

  in = fopen(fname, "rb");
  if (in == NULL) {
    perror(fname);
    exit(1);
  }
  read_header(in, &codec, &num_images, &format);

  for (i = 0; i < num_images; i++) {
    width = read_LEint32(in);
    height = read_LEint32(in);
    data = malloc(width * height * 2);

    if (codec == 0)
      read_data_codec0(in, width, height, data);
    else if (codec == 3) {
      size = read_LEint32(in);
      read_data_codec3(in, size, data);
    }
    else {
      fprintf(stderr, "%s: unsupported codec %d\n", fname, codec);
      exit(1);
    }

    if (format == 1) {
      img = toimg_fmt1(data, width, height);
      maxval = 255;
    }
    else if (format == 5) {
      img = toimg_fmt5(data, width, height);
      maxval = 255;
    }
    else {
      fprintf(stderr, "%s: unsupported format %d\n", fname, format);
      exit(1);
    }

    write_img(img, fname, i, width, height, maxval);
    free(data);
    ppm_freearray(img, height);
  }
  fclose(in);
}

int main(int argc, char **argv) {
  int i;

  ppm_init(&argc, argv);
  for (i = 1; i < argc; i++)
    process_file(argv[i]);
  return 0;
}
