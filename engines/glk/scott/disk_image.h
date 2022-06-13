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

int diRawnameFromName(byte *rawname, const char *name);

DiskImage *diCreateFromData(uint8_t *data, int length);

} // End of namespace Scott
} // End of namespace Glk

#endif
