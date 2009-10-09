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
 * $URL$
 * $Id$
 *
 */

#ifndef PSPLOADER_H
#define PSPLOADER_H

#include "elf32.h"
#include "common/list.h"
#include "common/singleton.h"

#define MAXDLERRLEN 80

#define ShortsMan	ShortSegmentManager::instance()

class ShortSegmentManager : public Common::Singleton<ShortSegmentManager> {
private:
	char *_shortsStart;
	char *_shortsEnd;

public:
	char *getShortsStart() {
		return _shortsStart;
	}
	bool inGeneralSegment(char *addr) {
		return ((char *)addr >= _shortsStart && (char *)addr < _shortsEnd);
	}

	class Segment {
	private:
		friend class ShortSegmentManager;
		Segment(char *start, int size, char *origAddr) : _startAddress(start), _size(size), _origAddress(origAddr) {}
		~Segment() {}
		char *_startAddress;		// Start of shorts segment in memory
		int  _size;					// Size of shorts segment
		char *_origAddress;			// Original address this segment was supposed to be at
	public:
		char *getStart() {
			return _startAddress;
		}
		char *getEnd() {
			return (_startAddress + _size);
		}
		Elf32_Addr getOffset() {
			return (Elf32_Addr)(_startAddress - _origAddress);
		}
		bool inSegment(char *addr) {
			return ((char *)addr >= _startAddress && (char *)addr < _startAddress + _size);
		}
	};

	Segment *newSegment(int size, char *origAddr);
	void deleteSegment(Segment *);

private:
	ShortSegmentManager();
	friend class Common::Singleton<ShortSegmentManager>;
	Common::List<Segment *> _list;
	char *_highestAddress;
};




class DLObject {
protected:
	char *_errbuf; /* For error messages, at least MAXDLERRLEN in size */

	ShortSegmentManager::Segment *_shortsSegment;			// For assigning shorts ranges
	void *_segment, *_symtab;
	char *_strtab;
	int _symbol_cnt;
	int _symtab_sect;
	void *_dtors_start, *_dtors_end;

	unsigned int _gpVal;		// Value of Global Pointer
	int _segmentSize;

	void seterror(const char *fmt, ...);
	void unload();
	bool relocate(int fd, unsigned long offset, unsigned long size, void *);
	bool load(int fd);

	bool readElfHeader(int fd, Elf32_Ehdr *ehdr);
	bool readProgramHeaders(int fd, Elf32_Ehdr *ehdr, Elf32_Phdr *phdr, int num);
	bool loadSegment(int fd, Elf32_Phdr *phdr);
	Elf32_Shdr *loadSectionHeaders(int fd, Elf32_Ehdr *ehdr);
	int loadSymbolTable(int fd, Elf32_Ehdr *ehdr, Elf32_Shdr *shdr);
	bool loadStringTable(int fd, Elf32_Shdr *shdr);
	void relocateSymbols(Elf32_Addr offset, Elf32_Addr shortsOffset);
	bool relocateRels(int fd, Elf32_Ehdr *ehdr, Elf32_Shdr *shdr);

public:
	bool open(const char *path);
	bool close();
	void *symbol(const char *name);
	void discard_symtab();

	DLObject(char *errbuf = NULL) : _errbuf(_errbuf), _shortsSegment(NULL), _segment(NULL), _symtab(NULL),
			_strtab(NULL), _symbol_cnt(0), _symtab_sect(-1), _dtors_start(NULL), _dtors_end(NULL), _gpVal(0) ,
			_segmentSize(0) {}
};



#define RTLD_LAZY 0

extern "C" {
	void *dlopen(const char *filename, int flags);
	int dlclose(void *handle);
	void *dlsym(void *handle, const char *symbol);
	const char *dlerror();
	void dlforgetsyms(void *handle);
}

#endif /* PSPLOADER_H */

