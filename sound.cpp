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

#include "stdafx.h"
#include "sound.h"
#include "bits.h"
#include "debug.h"
#include <cstring>
#include <SDL_endian.h>

#define ST_SAMPLE_MAX 0x7fffL
#define ST_SAMPLE_MIN (-ST_SAMPLE_MAX - 1L)

static inline void clampedAdd(int16& a, int b) {
        register int val = a + b;

        if (val > ST_SAMPLE_MAX)
                val = ST_SAMPLE_MAX;
        else if (val < ST_SAMPLE_MIN)
                val = ST_SAMPLE_MIN;

        a = val;
}

void vimaInit();
void decompressVima(const char *src, int16 *dest, int destLen);

void Sound::init() {
	vimaInit();
}

Sound::Sound(const char *filename, const char *data, int /* len */) :
  Resource(filename)
{
  const char *extension = filename + std::strlen(filename) - 3;
  const char *dataStart;
  int numBlocks, codecsLen;
  const char *codecsStart;
  const char *headerPos = data;
  int dataSize;

  if (strcasecmp(extension, "wav") == 0 || strcasecmp(extension, "imc") == 0) {
    // Read MCMP info
    if (std::memcmp(data, "MCMP", 4) != 0)
      error("Invalid file format in %s\n", filename);

    // The first block is the WAVE or IMUS header
    numBlocks = READ_BE_UINT16(data + 4);
    codecsStart = data + 8 + numBlocks * 9;
    codecsLen = READ_BE_UINT16(codecsStart - 2);
    headerPos = codecsStart + codecsLen;
  }

  if (strcasecmp(extension, "wav") == 0) {
    numChannels_ = READ_LE_UINT16(headerPos + 22);
    dataStart = headerPos + 28 + READ_LE_UINT32(headerPos + 16);
    dataSize = READ_LE_UINT32(dataStart - 4);
  }
  else if (strcasecmp(extension, "imc") == 0 ||
	   strcasecmp(extension, "imu") == 0) {
    // Ignore MAP info for now...
    if (memcmp(headerPos + 16, "FRMT", 4) != 0)
      error("FRMT block not where it was expected\n");
    numChannels_ = READ_BE_UINT32(headerPos + 40);
    dataStart = headerPos + 24 + READ_BE_UINT32(headerPos + 12);
    dataSize = READ_BE_UINT32(dataStart - 4);
  }
  else {
    error("Unrecognized extension for sound file %s\n", filename);
  }

  if (strcasecmp(extension, "wav") == 0 || strcasecmp(extension, "imc") == 0) {
    // Uncompress the samples
    numSamples_ = dataSize / 2;
    samples_ = new int16[dataSize / 2];
    int16 *destPos = samples_;
    const char *srcPos = dataStart;
    for (int i = 1; i < numBlocks; i++) { // Skip header block
      if (std::strcmp(codecsStart + 5 * *(uint8 *)(data + 6 + i * 9),
		      "VIMA") != 0)
	error("Unsupported codec %s\n",
	      codecsStart + 5 * *(uint8 *)(data + 6 + i * 9));
      decompressVima(srcPos, destPos, READ_BE_UINT32(data + 7 + i * 9));
      srcPos += READ_BE_UINT32(data + 11 + i * 9);
      destPos += READ_BE_UINT32(data + 7 + i * 9) / 2;
    }
  }
  else {
    numSamples_ = dataSize / 2;
    samples_ = new int16[dataSize / 2];
    std::memcpy(samples_, dataStart, dataSize);
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    for (int i = 0; i < numSamples_; i++)
      samples_[i] = SDL_Swap16(samples_[i]);
#endif
  }

  currPos_ = 0;
}

void Sound::reset() {
  currPos_ = 0;
}

void Sound::mix(int16 *data, int samples) {
  while (samples > 0 && currPos_ < numSamples_) {
    clampedAdd(*data, samples_[currPos_]);
    data++;
    if (numChannels_ == 1) {
      *data += samples_[currPos_];
      samples--;
      data++;
    }
    currPos_++;
    samples--;
  }
}

Sound::~Sound() {
  delete[] samples_;
}
