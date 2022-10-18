/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * https://paradroid.automac.se/diskimage/
 * Copyright (c) 2003-2006, Per Olofsson
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer. Redistributions in
 * binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef GLK_SCOTT_DISKIMAGE_H
#define GLK_SCOTT_DISKIMAGE_H

#include "common/scummsys.h"
#include "glk/scott/types.h"

namespace Glk {
namespace Scott {

// constants for the supported disk formats
#define MAXTRACKS 80
#define MAXSECTORS 40
#define D64SIZE 174848
#define D64ERRSIZE 175531
#define D71SIZE 349696
#define D71ERRSIZE 351062
#define D81SIZE 819200
#define D81ERRSIZE 822400

enum ImageType {
	D64 = 1,
	D71,
	D81
};

struct TrackSector {
	byte _track;
	byte _sector;
};

struct DiskImage {
	char *_filename;
	int _size;
	ImageType _type;
	byte *_image;
	byte *_errinfo;
	TrackSector _bam;
	TrackSector _bam2;
	TrackSector _dir;
	int _openfiles;
	int _blocksfree;
	int _modified;
	int _status;
	int _interleave;
	TrackSector _statusts;
};

struct RawDirEntry {
	TrackSector _nextts;
	byte _type;
	TrackSector _startts;
	byte _rawname[16];
	TrackSector _relsidets;
	byte _relrecsize;
	byte _unused[4];
	TrackSector _replacetemp;
	byte _sizelo;
	byte _sizehi;
};

struct ImageFile {
	DiskImage *_diskimage;
	RawDirEntry *_rawdirentry;
	char _mode;
	int _position;
	TrackSector _ts;
	TrackSector _nextts;
	byte *_buffer;
	int _bufptr;
	int _buflen;
	byte _visited[MAXTRACKS][MAXSECTORS];
};

ImageFile *diOpen(DiskImage *di, byte *rawname, byte type, const char *mode);
int diRead(ImageFile *imgfile, byte *buffer, int len);

byte *diGetTsAddr(DiskImage *di, TrackSector ts);
int diGetTsErr(DiskImage *di, TrackSector ts);

int diSectorsPerTrack(ImageType type, int track);
int diTracks(ImageType type);
int diGetBlockNum(ImageType type, TrackSector ts);

TrackSector diGetDirTs(DiskImage *di);
int diTrackBlocksFree(DiskImage *di, int track);
int diIsTsFree(DiskImage *di, TrackSector ts);
void diAllocTs(DiskImage *di, TrackSector ts);

int diRawnameFromName(byte *rawname, const char *name);

RawDirEntry *findLargestFileEntry(DiskImage *di);
DiskImage *diCreateFromData(uint8_t *data, int length);

} // End of namespace Scott
} // End of namespace Glk

#endif
