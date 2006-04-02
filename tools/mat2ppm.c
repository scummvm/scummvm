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

#include <ppm.h>

static pixel cmap[256];

int32_t read_LEint32(FILE *f) {
  unsigned char c[4];
  int i;

  fread(c, 1, 4, f);
  return (c[0]) | (c[1] << 8) | (c[2] << 16) | (c[3] << 24);
}

void read_cmp(const char *fname) {
  unsigned char col[3];
  FILE *cmp;
  int i;

  cmp = fopen(fname, "r");
  if (cmp == NULL) {
    perror(fname);
    exit(1);
  }
  fseek(cmp, 48, SEEK_SET);
  for (i = 0; i < 256; i++) {
    col[0] = getc(cmp); col[1] = getc(cmp); col[2] = getc(cmp);
    PPM_ASSIGN(cmap[i], col[0], col[1], col[2]);
  }
  fclose(cmp);
}

void write_img(FILE *f, const char *fname, int n, int num_img) {
  FILE *out;
  int32_t width, height;
  int x, y;
  unsigned char p;
  pixel **img;
  char newname[1024];
  const char *basename;

  fseek(f, 116 + 40 * (num_img - 1), SEEK_SET);
  width = read_LEint32(f);
  height = read_LEint32(f);
  img = ppm_allocarray(width, height);
  fseek(f, 100 + 40 * num_img + (width * height + 24) * n, SEEK_SET);
  for (y = 0; y < height; y++)
    for (x = 0; x < width; x++) {
      p = getc(f);
      img[y][x] = cmap[p];
    }

  basename = strrchr(fname, '/');
  if (basename != NULL)
    basename++;
  else
    basename = fname;
  strcpy(newname, basename);
  if (strlen(newname) > 4 &&
      strcasecmp(newname + strlen(newname) - 4, ".mat") == 0)
    newname[strlen(newname) - 4] = '\0';
  sprintf(newname + strlen(newname), "_%d.ppm", n);

  out = fopen(newname, "wb");
  if (out == NULL) {
    perror(newname);
    exit(1);
  }

  ppm_writeppm(out, img, width, height, 255, 0);
  ppm_freearray(img, height);
  fclose(out);
}

void process_file(const char *fname) {
  FILE *in;
  int32_t num_img;
  int i;

  in = fopen(fname, "rb");
  if (in == NULL) {
    perror(fname);
    exit(1);
  }
  fseek(in, 12, SEEK_SET);
  num_img = read_LEint32(in);
  for (i = 0; i < num_img; i++)
    write_img(in, fname, i, num_img);
  fclose(in);
}

int main(int argc, char **argv) {
  int i;

  ppm_init(&argc, argv);
  for (i = 0; i < 256; i++)
    PPM_ASSIGN(cmap[i], i, i, i);

  i = 1;
  if (argc >= 3 && strncmp(argv[1], "-c", 2) == 0) {
    read_cmp(argv[2]);
    i = 3;
  }
  for (; i < argc; i++)
    process_file(argv[i]);
  return 0;
}
