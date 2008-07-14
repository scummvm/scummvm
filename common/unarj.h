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

#ifndef COMMON_UNARJ_H
#define COMMON_UNARJ_H

#include "common/file.h"
#include "common/hash-str.h"

namespace Common {

#define HEADER_ID     0xEA60
#define HEADER_ID_HI    0xEA
#define HEADER_ID_LO    0x60
#define FIRST_HDR_SIZE    30
#define FIRST_HDR_SIZE_V  34
#define COMMENT_MAX     2048
#define FNAME_MAX           512
#define HEADERSIZE_MAX   (FIRST_HDR_SIZE + 10 + FNAME_MAX + COMMENT_MAX)
#define CRC_MASK        0xFFFFFFFFL
#define MAXSFX              25000L

#define CODE_BIT    16
#define CHAR_BIT  8
#define ARJ_UCHAR_MAX 255		// UCHAR_MAX is defined in limits.h in MSVC
#define THRESHOLD	3
#define DDICSIZ	  26624
#define MAXDICBIT   16
#define MATCHBIT	 8
#define MAXMATCH   256
#define NC		  (ARJ_UCHAR_MAX + MAXMATCH + 2 - THRESHOLD)
#define NP		  (MAXDICBIT + 1)
#define CBIT		 9
#define NT		  (CODE_BIT + 3)
#define PBIT		 5
#define TBIT		 5

#if NT > NP
#define NPT NT
#else
#define NPT NP
#endif

#define CTABLESIZE  4096
#define PTABLESIZE   256

#define STRTP		  9
#define STOPP		 13

#define STRTL		  0
#define STOPL		  7

struct ArjHeader {
	int32 pos;
	uint16 id;
	uint16 headerSize;
	//
	byte firstHdrSize;
	byte nbr;
	byte xNbr;
	byte hostOs;
	byte flags;
	byte method;
	byte fileType;
	byte pad;
	uint32 timeStamp;
	int32 compSize;
	int32 origSize;
	uint32 fileCRC;
	uint16 entryPos;
	uint16 fileMode;
	uint16 hostData;
	char   filename[FNAME_MAX];
	char   comment[COMMENT_MAX];
	uint16 extHeaderSize;

	uint32 headerCrc;
};

typedef HashMap<String, int, IgnoreCase_Hash, IgnoreCase_EqualTo> ArjFilesMap;

class ArjFile : public File {
public:
	ArjFile();
	~ArjFile();

	void enableFallback(bool val) { _fallBack = val; }

	void registerArchive(const String &filename);

	bool open(const Common::String &filename, AccessMode mode = kFileReadMode);
	void close();

	uint32 read(void *dataPtr, uint32 dataSize);
	bool eos();
	uint32 pos();
	uint32 size();
	void seek(int32 offset, int whence = SEEK_SET);
	bool isOpen() { return _isOpen; }

private:
	bool _fallBack;

	File _currArchive;
	Array<ArjHeader *> _headers;
	ArjFilesMap _fileMap;
	StringMap _archMap;
	ReadStream *_stream;
	byte *_uncompressedData;
	byte *_compressedData;
	MemoryWriteStream *_outstream;
	MemoryReadStream *_compressed;
	SeekableReadStream *_uncompressed;

	bool _isOpen;

	int32 findHeader(void);
	ArjHeader *readHeader();

	void decode();
	void decode_f();

	uint16 _bitbuf;
	int32 _compsize;
	int32 _origsize;
	byte _subbitbuf;
	int _bitcount;

	void init_getbits();
	void fillbuf(int n);
	uint16 getbits(int n);
	

	void make_table(int nchar, byte *bitlen, int tablebits, uint16 *table, int tablesize);
	void read_pt_len(int nn, int nbit, int i_special);
	void read_c_len(void);
	uint16 decode_c(void);
	uint16 decode_p(void);
	void decode_start(void);
	int16 decode_ptr(void);
	int16 decode_len(void);

private:
	byte  _text[DDICSIZ];

	int16  _getlen;
	int16  _getbuf;

	uint16 _left[2 * NC - 1];
	uint16 _right[2 * NC - 1];
	byte  _c_len[NC];
	byte  _pt_len[NPT];

	uint16 _c_table[CTABLESIZE];
	uint16 _pt_table[PTABLESIZE];
	uint16 _blocksize;


};

} // End of namespace Common

#endif
