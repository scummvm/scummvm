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

#include <cstdio>
#include <cstring>

int get_be_uint32(char *pos) {
  unsigned char *ucpos = reinterpret_cast<unsigned char *>(pos);
  return (ucpos[0] << 24) | (ucpos[1] << 16) | (ucpos[2] << 8) | ucpos[3];
}

void write_le_uint32(unsigned val) {
  putc(val, stdout);
  putc(val >> 8, stdout);
  putc(val >> 16, stdout);
  putc(val >> 24, stdout);
}

void write_le_uint16(unsigned short val) {
  putc(val, stdout);
  putc(val >> 8, stdout);
}

int main() {
  char block[1024];
  fread(block, 8, 1, stdin);	// skip iMUS header
  fread(block, 8, 1, stdin);	// read MAP header
  int mapSize = get_be_uint32(block + 4);
  int numBits = 16, rate = 22050, channels = 2;
  for (int mapPos = 0; mapPos < mapSize; ) {
    fread(block, 8, 1, stdin);
    int blockSize = get_be_uint32(block + 4);
    if (memcmp(block, "FRMT", 4) == 0) {
      fread(block, blockSize, 1, stdin);
      numBits = get_be_uint32(block + 8);
      rate = get_be_uint32(block + 12);
      channels = get_be_uint32(block + 16);
    }
    else
      fread(block, blockSize, 1, stdin);
    mapPos += (blockSize + 8);
  }
  fread(block, 8, 1, stdin);
  int dataSize = get_be_uint32(block + 4);
  fputs("RIFF", stdout);
  write_le_uint32(dataSize + 36);
  fputs("WAVEfmt ", stdout);
  write_le_uint32(16);
  write_le_uint16(1);
  write_le_uint16(channels);
  write_le_uint32(rate);
  write_le_uint32(channels * rate * (numBits / 8));
  write_le_uint16(channels * (numBits / 8));
  write_le_uint16(numBits);
  fputs("data", stdout);
  write_le_uint32(dataSize);
  while (dataSize > 1024) {
    fread(block, 1024, 1, stdin);
    fwrite(block, 1024, 1, stdout);
    dataSize -= 1024;
  }
  fread(block, dataSize, 1, stdin);
  fwrite(block, dataSize, 1, stdout);
  return 0;
}
