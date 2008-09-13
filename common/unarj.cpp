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
 * $URL$
 * $Id$
 *
 */

// Heavily based on Unarj 2.65

/* UNARJ.C, UNARJ, R JUNG, 06/05/02
 * Main Extractor routine
 * Copyright (c) 1991-2002 by ARJ Software, Inc.  All rights reserved.
 *
 *   This code may be freely used in programs that are NOT ARJ archivers
 *   (both compress and extract ARJ archives).
 *
 *   If you wish to distribute a modified version of this program, you
 *   MUST indicate that it is a modified version both in the program and
 *   source code.
 *
 *   We are holding the copyright on the source code, so please do not
 *   delete our name from the program files or from the documentation.
 *
 *   We wish to give credit to Haruhiko Okumura for providing the
 *   basic ideas for ARJ and UNARJ in his program AR.  Please note
 *   that UNARJ is significantly different from AR from an archive
 *   structural point of view.
 *
 */

#include "common/scummsys.h"
#include "common/util.h"
#include "common/unarj.h"

namespace Common {

static uint32 CRCtable[256];

static void	InitCRC(void) {
	const uint32 poly = 0xEDB88320;
	int i, j;
	uint32 n;

	for (i = 0; i < 256; i++) {
		n = i;
		for (j = 0; j < 8; j++)
			n = (n & 1) ? ((n >> 1) ^ poly) : (n >> 1);
		CRCtable[i] = n;
	}
}

static uint32 GetCRC(byte *data, int len) {
	uint32 CRC = 0xFFFFFFFF;
	int i;
	for (i = 0; i < len; i++)
		CRC = (CRC >> 8) ^ CRCtable[(CRC ^ data[i]) & 0xFF];
	return CRC ^ 0xFFFFFFFF;
}

ArjFile::ArjFile() : _uncompressedData(NULL) {
	InitCRC();
	_isOpen = false;
	_fallBack = false;
}

ArjFile::~ArjFile() {
	close();

	for (uint i = 0; i < _headers.size(); i++)
		delete _headers[i];

	_headers.clear();
	_fileMap.clear();
	_archMap.clear();
}

void ArjFile::registerArchive(const String &filename) {
	int32 first_hdr_pos;
	ArjHeader *header;

	if (!_currArchive.open(filename))
		return;

	first_hdr_pos = findHeader();

	if (first_hdr_pos < 0) {
		warning("ArjFile::registerArchive(): Could not find a valid header");
		return;
	}

	_currArchive.seek(first_hdr_pos, SEEK_SET);
	if (readHeader() == NULL)
		return;

	while ((header = readHeader()) != NULL) {
		_headers.push_back(header);

		_currArchive.seek(header->compSize, SEEK_CUR);

		_fileMap[header->filename] = _headers.size() - 1;
		_archMap[header->filename] = filename;
	}

	_currArchive.close();

	debug(0, "ArjFile::registerArchive(%s): Located %d files", filename.c_str(), _headers.size());
}

int32 ArjFile::findHeader(void) {
	long arcpos, lastpos;
	int c;
	byte header[HEADERSIZE_MAX];
	uint32 crc;
	uint16 headersize;

	arcpos = _currArchive.pos();
	_currArchive.seek(0L, SEEK_END);
	lastpos = _currArchive.pos() - 2;
	if (lastpos > MAXSFX)
		lastpos = MAXSFX;

	for ( ; arcpos < lastpos; arcpos++) {
		_currArchive.seek(arcpos, SEEK_SET);
		c = _currArchive.readByte();
		while (arcpos < lastpos) {
			if (c != HEADER_ID_LO)  // low order first
				c = _currArchive.readByte();
			else if ((c = _currArchive.readByte()) == HEADER_ID_HI)
				break;
			arcpos++;
		}
		if (arcpos >= lastpos)
			break;
		if ((headersize = _currArchive.readUint16LE()) <= HEADERSIZE_MAX) {
			_currArchive.read(header, headersize);
			crc = GetCRC(header, headersize);
			if (crc == _currArchive.readUint32LE()) {
				_currArchive.seek(arcpos, SEEK_SET);
				return arcpos;
			}
		}
	}
	return -1;		  // could not find a valid header
}

ArjHeader *ArjFile::readHeader() {
	ArjHeader header;
	ArjHeader *head;
	byte headData[HEADERSIZE_MAX];

	header.id = _currArchive.readUint16LE();
	if (header.id != HEADER_ID) {
		warning("ArjFile::readHeader(): Bad header ID (%x)", header.id);

		return NULL;
	}

	header.headerSize = _currArchive.readUint16LE();
	if (header.headerSize == 0)
		return NULL;			// end of archive
	if (header.headerSize > HEADERSIZE_MAX) {
		warning("ArjFile::readHeader(): Bad header");

		return NULL;
	}

	int rSize = _currArchive.read(headData, header.headerSize);

	MemoryReadStream readS(headData, rSize);

	header.headerCrc = _currArchive.readUint32LE();
	if (GetCRC(headData, header.headerSize) != header.headerCrc) {
		warning("ArjFile::readHeader(): Bad header CRC");
		return NULL;
	}

	header.firstHdrSize = readS.readByte();
	header.nbr = readS.readByte();
	header.xNbr = readS.readByte();
	header.hostOs = readS.readByte();
	header.flags = readS.readByte();
	header.method = readS.readByte();
	header.fileType = readS.readByte();
	(void)readS.readByte();
	header.timeStamp = readS.readUint32LE();
	header.compSize = readS.readSint32LE();
	header.origSize = readS.readSint32LE();
	header.fileCRC = readS.readUint32LE();
	header.entryPos = readS.readUint16LE();
	header.fileMode = readS.readUint16LE();
	header.hostData = readS.readUint16LE();

	if (header.origSize < 0 || header.compSize < 0) {
		warning("ArjFile::readHeader(): Wrong file size");
		return NULL;
	}

	strncpy(header.filename, (const char *)&headData[header.firstHdrSize], FNAME_MAX);

	strncpy(header.comment, (const char *)&headData[header.firstHdrSize + strlen(header.filename) + 1], COMMENT_MAX);

	/* if extheadersize == 0 then no CRC */
	/* otherwise read extheader data and read 4 bytes for CRC */

	while ((header.extHeaderSize = _currArchive.readUint16LE()) != 0)
		_currArchive.seek((long)(header.extHeaderSize + 4), SEEK_CUR);

	header.pos = _currArchive.pos();

	head = new ArjHeader(header);

	return head;
}


bool ArjFile::open(const Common::String &filename) {
	if (_isOpen)
		error("Attempt to open another instance of archive");

	_isOpen = false;

	if (_fallBack) {
		_currArchive.open(filename);
		if (_currArchive.isOpen()) {
			_isOpen = true;
			_uncompressed = &_currArchive;
			return true;
		}
	}

	if (!_fileMap.contains(filename))
		return false;

	_isOpen = true;

	ArjHeader *hdr = _headers[_fileMap[filename]];

	_compsize = hdr->compSize;
	_origsize = hdr->origSize;

	// FIXME: This hotfix prevents Drascula from leaking memory.
	// As far as sanity checks go this is not bad, but the engine should be fixed.
	if (_uncompressedData)
		free(_uncompressedData);

	_uncompressedData = (byte *)malloc(_origsize);
	_outstream = new MemoryWriteStream(_uncompressedData, _origsize);

	_currArchive.open(_archMap[filename]);
	_currArchive.seek(hdr->pos, SEEK_SET);

	if (hdr->method == 0) { // store
        _currArchive.read(_uncompressedData, _origsize);
	} else {
		_compressedData = (byte *)malloc(_compsize);
		_currArchive.read(_compressedData, _compsize);

		_compressed = new MemoryReadStream(_compressedData, _compsize);

		if (hdr->method == 1 || hdr->method == 2 || hdr->method == 3)
			decode();
		else if (hdr->method == 4)
			decode_f();

		delete _compressed;
		free(_compressedData);
	}

	_currArchive.close();
	delete _outstream;
	_outstream = NULL;

	_uncompressed = new MemoryReadStream(_uncompressedData, _origsize);

	return true;
}

void ArjFile::close() {
	if (!_isOpen)
		return;

	_isOpen = false;

	if (_fallBack) {
		_currArchive.close();
		return;
	} else {
		delete _uncompressed;
	}

	_uncompressed = NULL;

	free(_uncompressedData);
	_uncompressedData = NULL;
}

uint32 ArjFile::read(void *dataPtr, uint32 dataSize) {
	return _uncompressed->read(dataPtr, dataSize);
}

bool ArjFile::eos() {
	return _uncompressed->eos();
}

int32 ArjFile::pos() {
	return _uncompressed->pos();
}

int32 ArjFile::size() {
	return _uncompressed->size();
}

bool ArjFile::seek(int32 offset, int whence) {
	return _uncompressed->seek(offset, whence);
}

void ArjFile::init_getbits() {
	_bitbuf = 0;
	_subbitbuf = 0;
	_bitcount = 0;
	fillbuf(2 * CHAR_BIT);
}

void ArjFile::fillbuf(int n) {    // Shift bitbuf n bits left, read n bits
	_bitbuf = (_bitbuf << n) & 0xFFFF;  /* lose the first n bits */
	while (n > _bitcount) {
		_bitbuf |= _subbitbuf << (n -= _bitcount);
		if (_compsize != 0) {
			_compsize--;
			_subbitbuf = _compressed->readByte();
		} else
			_subbitbuf = 0;
		_bitcount = CHAR_BIT;
	}
	_bitbuf |= _subbitbuf >> (_bitcount -= n);
}

uint16 ArjFile::getbits(int n) {
	uint16 x;

	x = _bitbuf >> (2 * CHAR_BIT - n);
	fillbuf(n);
	return x;
}



/* Huffman decode routines */

void ArjFile::make_table(int nchar, byte *bitlen, int tablebits, uint16 *table, int tablesize) {
	uint16 count[17], weight[17], start[18], *p;
	uint i, k, len, ch, jutbits, avail, nextcode, mask;

	for (i = 1; i <= 16; i++)
		count[i] = 0;
	for (i = 0; (int)i < nchar; i++)
		count[bitlen[i]]++;

	start[1] = 0;
	for (i = 1; i <= 16; i++)
		start[i + 1] = start[i] + (count[i] << (16 - i));
	if (start[17] != (uint16) (1 << 16))
		error("ArjFile::make_table(): bad file data");

	jutbits = 16 - tablebits;
	for (i = 1; (int)i <= tablebits; i++) {
		start[i] >>= jutbits;
		weight[i] = 1 << (tablebits - i);
	}
	while (i <= 16) {
		weight[i] = 1 << (16 - i);
		i++;
	}

	i = start[tablebits + 1] >> jutbits;
	if (i != (uint16) (1 << 16)) {
		k = 1 << tablebits;
		while (i != k)
			table[i++] = 0;
	}

	avail = nchar;
	mask = 1 << (15 - tablebits);
	for (ch = 0; (int)ch < nchar; ch++) {
		if ((len = bitlen[ch]) == 0)
			continue;
		k = start[len];
		nextcode = k + weight[len];
		if ((int)len <= tablebits) {
			if (nextcode > (uint)tablesize)
				error("ArjFile::make_table(): bad file data");
			for (i = start[len]; i < nextcode; i++)
				table[i] = ch;
		} else {
			p = &table[k >> jutbits];
			i = len - tablebits;
			while (i != 0) {
				if (*p == 0) {
					_right[avail] = _left[avail] = 0;
					*p = avail++;
				}
				if (k & mask)
					p = &_right[*p];
				else
					p = &_left[*p];
				k <<= 1;
				i--;
			}
			*p = ch;
		}
		start[len] = nextcode;
	}
}

void ArjFile::read_pt_len(int nn, int nbit, int i_special) {
	int i, n;
	int16 c;
	uint16 mask;

	n = getbits(nbit);
	if (n == 0) {
		c = getbits(nbit);
		for (i = 0; i < nn; i++)
			_pt_len[i] = 0;
		for (i = 0; i < 256; i++)
			_pt_table[i] = c;
	} else {
		i = 0;
		while (i < n) {
			c = _bitbuf >> (13);
			if (c == 7) {
				mask = 1 << (12);
				while (mask & _bitbuf) {
					mask >>= 1;
					c++;
				}
			}
			fillbuf((c < 7) ? 3 : (int)(c - 3));
			_pt_len[i++] = (byte)c;
			if (i == i_special) {
				c = getbits(2);
				while (--c >= 0)
					_pt_len[i++] = 0;
			}
		}
		while (i < nn)
			_pt_len[i++] = 0;
		make_table(nn, _pt_len, 8, _pt_table, PTABLESIZE);  // replaced sizeof
	}
}

void ArjFile::read_c_len() {
	int16 i, c, n;
	uint16 mask;

	n = getbits(CBIT);
	if (n == 0) {
		c = getbits(CBIT);
		for (i = 0; i < NC; i++)
			_c_len[i] = 0;
		for (i = 0; i < CTABLESIZE; i++)
			_c_table[i] = c;
	} else {
		i = 0;
		while (i < n) {
			c = _pt_table[_bitbuf >> (8)];
			if (c >= NT) {
				mask = 1 << (7);
				do {
					if (_bitbuf & mask)
						c = _right[c];
					else
						c = _left[c];
					mask >>= 1;
				} while (c >= NT);
			}
			fillbuf((int)(_pt_len[c]));
			if (c <= 2) {
				if (c == 0)
					c = 1;
				else if (c == 1)
					c = getbits(4) + 3;
				else
					c = getbits(CBIT) + 20;
				while (--c >= 0)
					_c_len[i++] = 0;
			}
			else
				_c_len[i++] = (byte)(c - 2);
		}
		while (i < NC)
			_c_len[i++] = 0;
		make_table(NC, _c_len, 12, _c_table, CTABLESIZE);  // replaced sizeof
	}
}

uint16 ArjFile::decode_c() {
	uint16 j, mask;

	if (_blocksize == 0) {
		_blocksize = getbits(16);
		read_pt_len(NT, TBIT, 3);
		read_c_len();
		read_pt_len(NP, PBIT, -1);
	}
	_blocksize--;
	j = _c_table[_bitbuf >> 4];
	if (j >= NC) {
		mask = 1 << (3);
		do {
			if (_bitbuf & mask)
				j = _right[j];
			else
				j = _left[j];
			mask >>= 1;
		} while (j >= NC);
	}
	fillbuf((int)(_c_len[j]));
	return j;
}

uint16 ArjFile::decode_p() {
	uint16 j, mask;

	j = _pt_table[_bitbuf >> (8)];
	if (j >= NP) {
		mask = 1 << (7);
		do {
			if (_bitbuf & mask)
				j = _right[j];
			else
				j = _left[j];
			mask >>= 1;
		} while (j >= NP);
	}
	fillbuf((int)(_pt_len[j]));
	if (j != 0) {
		j--;
		j = (1 << j) + getbits((int)j);
	}
	return j;
}

void ArjFile::decode_start() {
	_blocksize = 0;
	init_getbits();
}

void ArjFile::decode() {
	int16 i;
	int16 j;
	int16 c;
	int16 r;
	int32 count;

	decode_start();
	count = 0;
	r = 0;

	while (count < _origsize) {
		if ((c = decode_c()) <= ARJ_UCHAR_MAX) {
			_text[r] = (byte) c;
			count++;
			if (++r >= DDICSIZ) {
				r = 0;
				_outstream->write(_text, DDICSIZ);
			}
		} else {
			j = c - (ARJ_UCHAR_MAX + 1 - THRESHOLD);
			count += j;
			i = decode_p();
			if ((i = r - i - 1) < 0)
				i += DDICSIZ;
			if (r > i && r < DDICSIZ - MAXMATCH - 1) {
				while (--j >= 0)
					_text[r++] = _text[i++];
			} else {
				while (--j >= 0) {
					_text[r] = _text[i];
					if (++r >= DDICSIZ) {
						r = 0;
						_outstream->write(_text, DDICSIZ);
					}
					if (++i >= DDICSIZ)
						i = 0;
				}
			}
		}
	}
	if (r != 0)
		_outstream->write(_text, r);
}

/* Macros */

#define BFIL {_getbuf|=_bitbuf>>_getlen;fillbuf(CODE_BIT-_getlen);_getlen=CODE_BIT;}
#define GETBIT(c) {if(_getlen<=0)BFIL c=(_getbuf&0x8000)!=0;_getbuf<<=1;_getlen--;}
#define BPUL(l) {_getbuf<<=l;_getlen-=l;}
#define GETBITS(c,l) {if(_getlen<l)BFIL c=(uint16)_getbuf>>(CODE_BIT-l);BPUL(l)}

int16 ArjFile::decode_ptr() {
	int16 c = 0;
	int16 width;
	int16 plus;
	int16 pwr;

	plus = 0;
	pwr = 1 << (STRTP);
	for (width = (STRTP); width < (STOPP); width++) {
		GETBIT(c);
		if (c == 0)
			break;
		plus += pwr;
		pwr <<= 1;
	}
	if (width != 0)
		GETBITS(c, width);
	c += plus;
	return c;
}

int16 ArjFile::decode_len() {
	int16 c = 0;
	int16 width;
	int16 plus;
	int16 pwr;

	plus = 0;
	pwr = 1 << (STRTL);
	for (width = (STRTL); width < (STOPL); width++) {
		GETBIT(c);
		if (c == 0)
			break;
		plus += pwr;
		pwr <<= 1;
	}
	if (width != 0)
		GETBITS(c, width);
	c += plus;
	return c;
}

void ArjFile::decode_f() {
	int16 i;
	int16 j;
	int16 c;
	int16 r;
	int16 pos1;
	int32 count;

	init_getbits();
	_getlen = _getbuf = 0;
	count = 0;
	r = 0;

	while (count < _origsize) {
		c = decode_len();
		if (c == 0) {
			GETBITS(c, CHAR_BIT);
			_text[r] = (byte)c;
			count++;
			if (++r >= DDICSIZ) {
				r = 0;
				_outstream->write(_text, DDICSIZ);
			}
		} else {
			j = c - 1 + THRESHOLD;
			count += j;
			pos1 = decode_ptr();
			if ((i = r - pos1 - 1) < 0)
				i += DDICSIZ;
			while (j-- > 0) {
				_text[r] = _text[i];
				if (++r >= DDICSIZ) {
					r = 0;
					_outstream->write(_text, DDICSIZ);
				}
				if (++i >= DDICSIZ)
					i = 0;
			}
		}
	}
	if (r != 0)
		_outstream->write(_text, r);
}


} // End of namespace Common
