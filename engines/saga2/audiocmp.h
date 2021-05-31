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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_AUDIOCMP_H
#define SAGA2_AUDIOCMP_H

namespace Saga2 {

/* ===================================================================== *
    Tweak values
 * ===================================================================== */

#define MAXIMAL_COMPRESSION 8
#define EXPECTED_COMPRESSION 4

/* ===================================================================== *
    Decompression engine
 * ===================================================================== */

class soundDecompression {
public:

	workBuffer  *filei;      // input 'file' ( buffer )
	workBuffer *workBuf;     // input 'file' ( buffer ) copy
	Buffer  *fileo;          // output 'file' ( buffer )
	Buffer *targBuf;         // output 'file' ( buffer ) copy

	soundDecoder *readFrom;    // input decoder
	soundSample *sampleAttrib; // audio attributes

	uint32 rVal;               // decoder return value

	long **buffer, * *offset;
	long default_offset, lpcqoffset;
	int version, bitshift ;
	int hiloint, hilo ;
	int ftype ;
	int blocksize, nchan;
	int i, chan, nwrap, nskip, ndiscard ;
	int *qlpc, maxnlpc, nmean ;
	int quanterror, nfilename;
	int minsnr;
	int decompState;
	char *magic, *old_magic, *filenamei, *filenameo ;
	char *tmpfilename ;
	char *maxresnstr;

	bool decerr, deceof;       // error, eof detect on input buffer
	bool deserr, deseof;       // error, eof detect on output buffer

	soundDecompression(void);
	soundDecompression(Buffer *sb, soundDecoder *sd, soundSample *ss, workBuffer *wb);
	void *operator new (size_t s) {
		return audioAlloc(s, "audio decmp data");
	}
	void operator delete (void *m) {
		audioFree(m);
	}

	void reset(void);              // initializefor new 'file'
	void setFType(int16 dft);     // set the file type

	size_t readb(Buffer *sb, Buffer *tb);
	bool openb(Buffer *sb, Buffer *tb);

	bool beof(void) {
		return deceof;
	}
	bool berr(void) {
		return decerr;
	}
	bool weof(void) {
		return fileo->laden();
	}
	bool werr(void) {
		return deserr;
	}

};

/* ===================================================================== *
    File Emulation for buffers
 * ===================================================================== */

Buffer *buffopen(char n[], char att[]);           // emulate fopen
size_t buffread(void *, size_t, size_t, Buffer *);   // emulate fread
int buffgetc(Buffer *);                           // emulate fgetc
char buffputc(char, Buffer *);                     // emulate fputc
int16 buffclose(Buffer *);                        // emulate fclose
size_t buffwrite(void *, size_t, size_t, Buffer *);  // emulate fwrite

} // end of namespace Saga2

#endif  //DECOMP_H
