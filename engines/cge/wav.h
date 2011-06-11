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

/*
 * This code is based on original Soltys source code
 * Copyright (c) 1994-1995 Janus B. Wisniewski and L.K. Avalon
 */

#ifndef	__WAV__
#define	__WAV__

#include	"cge/general.h"
#include	<string.h>

namespace CGE {

#define		WAVE_FORMAT_PCM		0x0001
#define		IBM_FORMAT_MULAW	0x0101
#define		IBM_FORMAT_ALAW		0x0102
#define		IBM_FORMAT_ADPCM	0x0103



typedef char FOURCC[4];				// Four-character code
typedef uint32 CKSIZE;				// 32-bit unsigned size


class CKID // Chunk type identifier
{
  union { FOURCC Tx; uint32 Id; };
protected:
  static XFILE * ckFile;
public:
  CKID (FOURCC t) { memcpy(Tx, t, sizeof(Tx)); }
  CKID (uint32 d) { Id = d; }
  CKID (XFILE * xf) { (ckFile = xf)->Read(Tx, sizeof(Tx)); }
  bool operator !=(CKID& X) { return Id != X.Id; }
  bool operator ==(CKID& X) { return Id == X.Id; }
  const char * Name (void);
};




class CKHEA : public CKID
{
protected:
  CKSIZE ckSize;		// Chunk size field (size of ckData)
public:
  CKHEA (XFILE * xf) : CKID(xf) { XRead(xf, &ckSize); }
  CKHEA (char id[]) : CKID(id), ckSize(0) { }
  void Skip (void);
  CKSIZE Size (void) { return ckSize; }
};





class FMTCK : public CKHEA
{
  struct WAV
  {
    uint16  wFormatTag;         // Format category
    uint16  wChannels;          // Number of channels
    uint32 dwSamplesPerSec;    // Sampling rate
    uint32 dwAvgBytesPerSec;   // For buffer estimation
    uint16  wBlockAlign;        // Data block size
  } Wav;

  union
  {
    struct PCM
    {
      uint16 wBitsPerSample;      // Sample size
    } Pcm;
  };
public:
  FMTCK (CKHEA& hea);
  inline  uint16 Channels (void) { return Wav.wChannels; }
  inline uint32 SmplRate (void) { return Wav.dwSamplesPerSec; }
  inline uint32 ByteRate (void) { return Wav.dwAvgBytesPerSec; }
  inline  uint16 BlckSize (void) { return Wav.wBlockAlign; }
  inline  uint16 SmplSize (void) { return Pcm.wBitsPerSample; }
};





class DATACK : public CKHEA
{
  bool e;
  union
    {
      uint8 * Buf;
      EMS * EBuf;
    };
public:
  DATACK (CKHEA& hea);
  DATACK (CKHEA& hea, EMM * emm);
  DATACK (int first, int last);
  ~DATACK (void);
  inline uint8 * Addr (void) { return Buf; }
  inline EMS * EAddr (void) { return EBuf; }
};



extern	CKID	RIFF;
extern	CKID	WAVE;
extern	CKID	FMT;
extern	CKID	DATA;


DATACK *	LoadWave	(XFILE * file, EMM * emm = NULL);


} // End of namespace CGE


#endif
