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

#define ARJ_UCHAR_MAX 255
#define ARJ_CHAR_BIT 8

#define ARJ_COMMENT_MAX 2048
#define ARJ_FILENAME_MAX 512

#define ARJ_CODE_BIT 16
#define ARJ_THRESHOLD 3
#define ARJ_DICSIZ 26624
#define ARJ_FDICSIZ ARJ_DICSIZ
#define ARJ_MAXDICBIT   16
#define ARJ_MAXMATCH   256
#define ARJ_NC (ARJ_UCHAR_MAX + ARJ_MAXMATCH + 2 - ARJ_THRESHOLD)
#define ARJ_NP (ARJ_MAXDICBIT + 1)
#define ARJ_NT (ARJ_CODE_BIT + 3)

#if ARJ_NT > ARJ_NP
#define ARJ_NPT ARJ_NT
#else
#define ARJ_NPT ARJ_NP
#endif

#define ARJ_CTABLESIZE 4096
#define ARJ_PTABLESIZE 256


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
	char   filename[ARJ_FILENAME_MAX];
	char   comment[ARJ_COMMENT_MAX];

	uint32 headerCrc;
};

typedef HashMap<String, int, IgnoreCase_Hash, IgnoreCase_EqualTo> ArjFilesMap;

class ArjFile : public SeekableReadStream, public NonCopyable {
public:
	ArjFile();
	~ArjFile();

	void enableFallback(bool val) { _fallBack = val; }

	void registerArchive(const String &filename);

	bool open(const Common::String &filename);
	void close();

	uint32 read(void *dataPtr, uint32 dataSize);
	bool eos() const;
	int32 pos() const;
	int32 size() const;
	bool seek(int32 offset, int whence = SEEK_SET);
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
	uint16 _bytebuf;
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
	byte  _ntext[ARJ_FDICSIZ];

	int16  _getlen;
	int16  _getbuf;

	uint16 _left[2 * ARJ_NC - 1];
	uint16 _right[2 * ARJ_NC - 1];
	byte  _c_len[ARJ_NC];
	byte  _pt_len[ARJ_NPT];

	uint16 _c_table[ARJ_CTABLESIZE];
	uint16 _pt_table[ARJ_PTABLESIZE];
	uint16 _blocksize;


};

} // End of namespace Common

#endif
