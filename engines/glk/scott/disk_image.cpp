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

#include "common/scummsys.h"
#include "common/str.h"
#include "common/textconsole.h"
#include "glk/scott/disk_image.h"

namespace Glk {
namespace Scott {

/* dos errors as used by the DOS internally (and as saves in the error info) */
struct DosError {
	signed int _number; /* dos error number */
	signed int _errnum; /* reported error number */
	const char *_string;      /* description */
};

DosError g_dosError[] = {
	/* non-errors */
	{0x01, 0, "ok"},
	/* errors */
	{0x02, 20, "Header descriptor byte not found (Seek)"},
	/*  { 0x02, 20, "Header block not found (Seek)" }, */
	{0x03, 21, "No SYNC sequence found (Seek)"},
	{0x04, 22, "Data descriptor byte not found (Read)"},
	{0x05, 23, "Checksum error in data block (Read)"},
	{0x06, 24, "Write verify (Write)"},
	{0x07, 25, "Write verify error (Write)"},
	{0x08, 26, "Write protect on (Write)"},
	{0x09, 27, "Checksum error in header block (Seek)"},
	{0x0A, 28, "Write error (Write)"},
	{0x0B, 29, "Disk sector ID mismatch (Seek)"},
	{0x0F, 74, "Drive Not Ready (Read)"},
	{-1, -1, nullptr}
};

int setStatus(DiskImage *di, int status, int track, int sector) {
	di->_status = status;
	di->_statusts._track = track;
	di->_statusts._sector = sector;
	return status;
}

/* check if given track/sector is within valid range */
int diTsIsValid(ImageType type, TrackSector ts) {
	if ((ts._track < 1) || (ts._track > diTracks(type))) {
		return 0; /* track out of range */
	}
	if (ts._sector > (diSectorsPerTrack(type, ts._track) - 1)) {
		return 0; /* sector out of range */
	}
	return 1;
}

int matchPattern(byte *rawPattern, byte *rawname) {
	int i;

	for (i = 0; i < 16; ++i) {
		if (rawPattern[i] == '*') {
			return 1;
		}
		if (rawname[i] == 0xa0) {
			if (rawPattern[i] == 0xa0) {
				return 1;
			} else {
				return 0;
			}
		} else {
			if (rawPattern[i] == '?' || rawPattern[i] == rawname[i]) {
			} else {
				return 0;
			}
		}
	}
	return 1;
}

/* return a pointer to the next block in the chain */
TrackSector nextTsInChain(DiskImage *di, TrackSector ts) {
	byte *p;
	TrackSector newTs;

	p = diGetTsAddr(di, ts);
	newTs._track = p[0];
	newTs._sector = p[1];

	return newTs;
}

RawDirEntry *findFileEntry(DiskImage *di, byte *rawPattern, int type) {
	byte *buffer;
	TrackSector ts;
	RawDirEntry *rde;
	int offset;

	ts = diGetDirTs(di);

	while (ts._track) {
		buffer = diGetTsAddr(di, ts);
		for (offset = 0; offset < 256; offset += 32) {
			rde = (RawDirEntry *)(buffer + offset);
			if (matchPattern(rawPattern, rde->_rawname)) {
				return rde;
			}
		}
		/* todo: add sanity checking */
		ts = nextTsInChain(di, ts);
	}
	return nullptr;
}

/* allocate next available directory block */
TrackSector allocNextDirTs(DiskImage *di) {
	byte *p;
	int spt;
	TrackSector ts, lastTs;

	if (diTrackBlocksFree(di, di->_bam._track)) {
		lastTs._track = di->_bam._track;
		lastTs._sector = 0;
		if ((di->_type == D64) || (di->_type == D71)) {
			ts._track = 18;
			ts._sector = 1;
		} else {
			ts = nextTsInChain(di, lastTs);
		}
		while (ts._track) {
			lastTs = ts;
			ts = nextTsInChain(di, ts);
		}
		ts._track = lastTs._track;
		ts._sector = lastTs._sector + 3;
		spt = diSectorsPerTrack(di->_type, ts._track);
		for (;; ts._sector = (ts._sector + 1) % spt) {
			if (diIsTsFree(di, ts)) {
				diAllocTs(di, ts);
				p = diGetTsAddr(di, lastTs);
				p[0] = ts._track;
				p[1] = ts._sector;
				p = diGetTsAddr(di, ts);
				memset(p, 0, 256);
				p[1] = 0xff;
				di->_modified = 1;
				return ts;
			}
		}
	} else {
		ts._track = 0;
		ts._sector = 0;
		return ts;
	}
}

RawDirEntry *allocFileEntry(DiskImage *di, byte *rawName, int type) {
	byte *buffer;
	TrackSector ts;
	RawDirEntry *rde;
	int offset;

	/* check if file already exists */
	ts = nextTsInChain(di, di->_bam);
	while (ts._track) {
		buffer = diGetTsAddr(di, ts);
		for (offset = 0; offset < 256; offset += 32) {
			rde = (RawDirEntry *)(buffer + offset);
			if (rde->_type) {
				if (scumm_strnicmp((char *)rawName, (char *)rde->_rawname, 16) == 0) {
					setStatus(di, 63, 0, 0);
					return nullptr;
				}
			}
		}
		ts = nextTsInChain(di, ts);
	}

	/* allocate empty slot */
	ts = nextTsInChain(di, di->_bam);
	while (ts._track) {
		buffer = diGetTsAddr(di, ts);
		for (offset = 0; offset < 256; offset += 32) {
			rde = (RawDirEntry *)(buffer + offset);
			if (rde->_type == 0) {
				memset((byte *)rde + 2, 0, 30);
				memcpy(rde->_rawname, rawName, 16);
				rde->_type = type;
				di->_modified = 1;
				return rde;
			}
		}
		ts = nextTsInChain(di, ts);
	}

	/* allocate new dir block */
	ts = allocNextDirTs(di);
	if (ts._track) {
		rde = (RawDirEntry *)diGetTsAddr(di, ts);
		memset((byte *)rde + 2, 0, 30);
		memcpy(rde->_rawname, rawName, 16);
		rde->_type = type;
		di->_modified = 1;
		return rde;
	} else {
		setStatus(di, 72, 0, 0);
		return nullptr;
	}
}

ImageFile *diOpen(DiskImage *di, byte *rawname, byte type, const char *mode) {
	ImageFile *imgFile;
	RawDirEntry *rde;
	byte *p;

	setStatus(di, 255, 0, 0);

	if (scumm_stricmp("rb", mode) == 0) {

		if ((imgFile = new ImageFile) == nullptr) {
			return nullptr;
		}

		memset(imgFile->_visited, 0, sizeof(imgFile->_visited));

		if (scumm_stricmp("$", (char *)rawname) == 0) {
			imgFile->_mode = 'r';

			imgFile->_ts = di->_dir;

			p = diGetTsAddr(di, di->_dir);
			imgFile->_buffer = p + 2;

			imgFile->_nextts = diGetDirTs(di);

			imgFile->_buflen = 254;

			if (!diTsIsValid(di->_type, imgFile->_nextts)) {
				setStatus(di, 66, imgFile->_nextts._track, imgFile->_nextts._sector);
				delete imgFile;
				return nullptr;
			}
			rde = nullptr;

		} else {
			if ((rde = findFileEntry(di, rawname, type)) == nullptr) {
				setStatus(di, 62, 0, 0);
				delete imgFile;
				return nullptr;
			}
			imgFile->_mode = 'r';
			imgFile->_ts = rde->_startts;

			if (!diTsIsValid(di->_type, imgFile->_ts)) {
				setStatus(di, 66, imgFile->_ts._track, imgFile->_ts._sector);
				delete imgFile;
				return nullptr;
			}

			p = diGetTsAddr(di, rde->_startts);
			imgFile->_buffer = p + 2;
			imgFile->_nextts._track = p[0];
			imgFile->_nextts._sector = p[1];

			if (imgFile->_nextts._track == 0) {
				if (imgFile->_nextts._sector != 0) {
					imgFile->_buflen = imgFile->_nextts._sector - 1;
				} else {
					imgFile->_buflen = 254;
				}
			} else {
				if (!diTsIsValid(di->_type, imgFile->_nextts)) {
					setStatus(di, 66, imgFile->_nextts._track, imgFile->_nextts._sector);
					delete imgFile;
					return nullptr;
				}
				imgFile->_buflen = 254;
			}
		}

	} else if (strcmp("wb", mode) == 0) {

		if ((rde = allocFileEntry(di, rawname, type)) == nullptr) {
			return nullptr;
		}
		if ((imgFile = new ImageFile) == nullptr) {
			return nullptr;
		}
		imgFile->_mode = 'w';
		imgFile->_ts._track = 0;
		imgFile->_ts._sector = 0;
		if ((imgFile->_buffer = new byte[254]) == nullptr) {
			delete imgFile;
			return nullptr;
		}
		imgFile->_buflen = 254;
		di->_modified = 1;

	} else {
		return nullptr;
	}

	imgFile->_diskimage = di;
	imgFile->_rawdirentry = rde;
	imgFile->_position = 0;
	imgFile->_bufptr = 0;

	++(di->_openfiles);
	setStatus(di, 0, 0, 0);
	return imgFile;
}

int diRead(ImageFile *imgFile, byte *buffer, int len) {
	byte *p;
	int bytesLeft;
	int counter = 0;
	int err;

	while (len) {
		bytesLeft = imgFile->_buflen - imgFile->_bufptr;

		err = diGetTsErr(imgFile->_diskimage, imgFile->_ts);
		if (err) {
			setStatus(imgFile->_diskimage, err, imgFile->_ts._track, imgFile->_ts._sector);
			return counter;
		}

		if (bytesLeft == 0) {
			if (imgFile->_nextts._track == 0) {
				return counter;
			}
			if (((imgFile->_diskimage->_type == D64) || (imgFile->_diskimage->_type == D71)) && imgFile->_ts._track == 18 && imgFile->_ts._sector == 0) {
				imgFile->_ts._track = 18;
				imgFile->_ts._sector = 1;
			} else {
				imgFile->_ts = nextTsInChain(imgFile->_diskimage, imgFile->_ts);
			}
			if (imgFile->_ts._track == 0) {
				return counter;
			}

			/* check for cyclic files */
			if (imgFile->_visited[imgFile->_ts._track][imgFile->_ts._sector]) {
				/* return 52, file too long error */
				setStatus(imgFile->_diskimage, 52, imgFile->_ts._track, imgFile->_ts._sector);
			} else {
				imgFile->_visited[imgFile->_ts._track][imgFile->_ts._sector] = 1;
			}

			err = diGetTsErr(imgFile->_diskimage, imgFile->_ts);
			if (err) {
				setStatus(imgFile->_diskimage, err, imgFile->_ts._track, imgFile->_ts._sector);
				return counter;
			}

			p = diGetTsAddr(imgFile->_diskimage, imgFile->_ts);
			imgFile->_buffer = p + 2;
			imgFile->_nextts._track = p[0];
			imgFile->_nextts._sector = p[1];

			if (imgFile->_nextts._track == 0) {
				if (imgFile->_nextts._sector == 0) {
					/* fixme, something is wrong if this happens, should be a proper error */
					imgFile->_buflen = 0;
					setStatus(imgFile->_diskimage, -1, imgFile->_ts._track, imgFile->_ts._sector);
				} else {
					imgFile->_buflen = imgFile->_nextts._sector - 1;
				}
			} else {
				if (!diTsIsValid(imgFile->_diskimage->_type, imgFile->_nextts)) {
					setStatus(imgFile->_diskimage, 66, imgFile->_nextts._track, imgFile->_nextts._sector);
					return counter;
				}
				imgFile->_buflen = 254;
			}
			imgFile->_bufptr = 0;
		} else {
			if (len >= bytesLeft) {
				while (bytesLeft) {
					*buffer++ = imgFile->_buffer[imgFile->_bufptr++];
					--len;
					--bytesLeft;
					++counter;
					++(imgFile->_position);
				}
			} else {
				while (len) {
					*buffer++ = imgFile->_buffer[imgFile->_bufptr++];
					--len;
					--bytesLeft;
					++counter;
					++(imgFile->_position);
				}
			}
		}
	}
	return counter;
}

// get a pointer to block data 
byte *diGetTsAddr(DiskImage *di, TrackSector ts) {
	return di->_image + diGetBlockNum(di->_type, ts) * 256;
}

/* get error info for a sector */
int getTsDosErr(DiskImage *di, TrackSector ts) {
	//	return 1;
	if (di->_errinfo == nullptr) {
		return 1; /* return OK if image has no error info */
	}

	return di->_errinfo[diGetBlockNum(di->_type, ts)];
}

int diGetTsErr(DiskImage *di, TrackSector ts) {
	int errnum;
	DosError *err = g_dosError;

	errnum = getTsDosErr(di, ts);
	while (err->_number >= 0) {
		if (errnum == err->_number) {
			return err->_errnum;
		}
		++err;
	}
	return -1; /* unknown error */
}

/* return disk geometry for track */
int diSectorsPerTrack(ImageType type, int track) {
	switch (type) {
	case D71:
		if (track > 35) {
			track -= 35;
		}
		/* fall through */
	case D64:
		if (track < 18) {
			return 21;
		} else if (track < 25) {
			return 19;
		} else if (track < 31) {
			return 18;
		} else {
			return 17;
		}
		break;
	case D81:
		return 40;
		break;
	}
	return 0;
}

/* return number of tracks for image type */
int diTracks(ImageType type) {
	switch (type) {
	case D64:
		return 35;
		break;
	case D71:
		return 70;
		break;
	case D81:
		return 80;
		break;
	}
	return 0;
}

int diGetBlockNum(ImageType type, TrackSector ts) {
	int block;

	/* assertion, should never happen (indicates bad error handling elsewhere) */
	if (!diTsIsValid(type, ts)) {
		error("diGetBlockNum: internal error, track/sector out of range");
	}

	switch (type) {
	case D64:
		if (ts._track < 18) {
			block = (ts._track - 1) * 21;
		} else if (ts._track < 25) {
			block = (ts._track - 18) * 19 + 17 * 21;
		} else if (ts._track < 31) {
			block = (ts._track - 25) * 18 + 17 * 21 + 7 * 19;
		} else {
			block = (ts._track - 31) * 17 + 17 * 21 + 7 * 19 + 6 * 18;
		}
		return block + ts._sector;
		break;
	case D71:
		if (ts._track > 35) {
			block = 683;
			ts._track -= 35;
		} else {
			block = 0;
		}
		if (ts._track < 18) {
			block += (ts._track - 1) * 21;
		} else if (ts._track < 25) {
			block += (ts._track - 18) * 19 + 17 * 21;
		} else if (ts._track < 31) {
			block += (ts._track - 25) * 18 + 17 * 21 + 7 * 19;
		} else {
			block += (ts._track - 31) * 17 + 17 * 21 + 7 * 19 + 6 * 18;
		}
		return block + ts._sector;
		break;
	case D81:
		return (ts._track - 1) * 40 + ts._sector;
		break;
	}
	return 0;
}

/* return t/s of first directory sector */
TrackSector diGetDirTs(DiskImage *di) {
	TrackSector newTs;
	byte *p;

	p = diGetTsAddr(di, di->_dir);
	if ((di->_type == D64) || (di->_type == D71)) {
		newTs._track = 18; /* 1541/71 ignores bam t/s link */
		newTs._sector = 1;
	} else {
		newTs._track = p[0];
		newTs._sector = p[1];
	}

	return newTs;
}

/* return number of free blocks in track */
int diTrackBlocksFree(DiskImage *di, int track) {
	byte *bam;

	switch (di->_type) {
	default:
	case D64:
		bam = diGetTsAddr(di, di->_bam);
		break;
	case D71:
		bam = diGetTsAddr(di, di->_bam);
		if (track >= 36) {
			return bam[track + 185];
		}
		break;
	case D81:
		if (track <= 40) {
			bam = diGetTsAddr(di, di->_bam);
		} else {
			bam = diGetTsAddr(di, di->_bam2);
			track -= 40;
		}
		return bam[track * 6 + 10];
		break;
	}
	return bam[track * 4];
}

/* check if track, sector is free in BAM */
int diIsTsFree(DiskImage *di, TrackSector ts) {
	byte mask;
	byte *bam;

	switch (di->_type) {
	case D64:
		bam = diGetTsAddr(di, di->_bam);
		if (bam[ts._track * 4]) {
			mask = 1 << (ts._sector & 7);
			return bam[ts._track * 4 + ts._sector / 8 + 1] & mask ? 1 : 0;
		} else {
			return 0;
		}
		break;
	case D71:
		mask = 1 << (ts._sector & 7);
		if (ts._track < 36) {
			bam = diGetTsAddr(di, di->_bam);
			return bam[ts._track * 4 + ts._sector / 8 + 1] & mask ? 1 : 0;
		} else {
			bam = diGetTsAddr(di, di->_bam2);
			return bam[(ts._track - 35) * 3 + ts._sector / 8 - 3] & mask ? 1 : 0;
		}
		break;
	case D81:
		mask = 1 << (ts._sector & 7);
		if (ts._track < 41) {
			bam = diGetTsAddr(di, di->_bam);
		} else {
			bam = diGetTsAddr(di, di->_bam2);
			ts._track -= 40;
		}
		return bam[ts._track * 6 + ts._sector / 8 + 11] & mask ? 1 : 0;
		break;
	}
	return 0;
}

/* allocate track, sector in BAM */
void diAllocTs(DiskImage *di, TrackSector ts) {
	byte mask;
	byte *bam;

	di->_modified = 1;
	switch (di->_type) {
	case D64:
		bam = diGetTsAddr(di, di->_bam);
		bam[ts._track * 4] -= 1;
		mask = 1 << (ts._sector & 7);
		bam[ts._track * 4 + ts._sector / 8 + 1] &= ~mask;
		break;
	case D71:
		mask = 1 << (ts._sector & 7);
		if (ts._track < 36) {
			bam = diGetTsAddr(di, di->_bam);
			bam[ts._track * 4] -= 1;
			bam[ts._track * 4 + ts._sector / 8 + 1] &= ~mask;
		} else {
			bam = diGetTsAddr(di, di->_bam);
			bam[ts._track + 185] -= 1;
			bam = diGetTsAddr(di, di->_bam2);
			bam[(ts._track - 35) * 3 + ts._sector / 8 - 3] &= ~mask;
		}
		break;
	case D81:
		if (ts._track < 41) {
			bam = diGetTsAddr(di, di->_bam);
		} else {
			bam = diGetTsAddr(di, di->_bam2);
			ts._track -= 40;
		}
		bam[ts._track * 6 + 10] -= 1;
		mask = 1 << (ts._sector & 7);
		bam[ts._track * 6 + ts._sector / 8 + 11] &= ~mask;
		break;
	}
}

/* convert to rawname */
int diRawnameFromName(byte *rawname, const char *name) {
	int i;

	memset(rawname, 0xa0, 16);
	for (i = 0; i < 16 && name[i]; ++i)
		rawname[i] = name[i];
	return i;
}

/* count number of free blocks */
int blocksFree(DiskImage *di) {
	int blocks = 0;

	for (int track = 1; track <= diTracks(di->_type); ++track) {
		if (track != di->_dir._track) {
			blocks += diTrackBlocksFree(di, track);
		}
	}
	return blocks;
}

/* return write interleave */
int interleave(ImageType type) {
	switch (type) {
	case D64:
		return 10;
		break;
	case D71:
		return 6;
		break;
	default:
		return 1;
		break;
	}
}

DiskImage *diCreateFromData(uint8_t *data, int length) {
	DiskImage *di;

	if ((di = new DiskImage) == nullptr) {
		return nullptr;
	}

	di->_size = length;

	/* allocate buffer for image */
	if ((di->_image = new byte[length]) == nullptr) {
		delete di;
		return nullptr;
	}

	di->_image = data;

	di->_errinfo = nullptr;

	/* check image type */
	switch (length) {
	case D64ERRSIZE: /* D64 with error info */
		// di->_errinfo = &(di->_error_);
	case D64SIZE: /* standard D64 */
		di->_type = D64;
		di->_bam._track = 18;
		di->_bam._sector = 0;
		di->_dir = di->_bam;
		break;

	case D71ERRSIZE: /* D71 with error info */
		di->_errinfo = &(di->_image[D71SIZE]);
	case D71SIZE:
		di->_type = D71;
		di->_bam._track = 18;
		di->_bam._sector = 0;
		di->_bam2._track = 53;
		di->_bam2._sector = 0;
		di->_dir = di->_bam;
		break;

	case D81ERRSIZE: /* D81 with error info */
		di->_errinfo = &(di->_image[D81SIZE]);
	case D81SIZE:
		di->_type = D81;
		di->_bam._track = 40;
		di->_bam._sector = 1;
		di->_bam2._track = 40;
		di->_bam2._sector = 2;
		di->_dir._track = 40;
		di->_dir._sector = 0;
		break;

	default:
		delete[] di->_image;
		delete di;
		return nullptr;
	}

	di->_filename = nullptr;
	di->_openfiles = 0;
	di->_blocksfree = blocksFree(di);
	di->_modified = 0;
	di->_interleave = interleave(di->_type);
	setStatus(di, 254, 0, 0);
	return di;
}

} // End of namespace Scott
} // End of namespace Glk
