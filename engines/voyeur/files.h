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

#ifndef VOYEUR_FILES_H
#define VOYEUR_FILES_H

#include "common/scummsys.h"
#include "common/file.h"
#include "common/rect.h"
#include "common/str.h"
#include "voyeur/graphics.h"

namespace Voyeur {

class VoyeurEngine;
class BoltFile;
class BoltGroup;
class BoltEntry;
class PictureResource;
class ViewPortResource;
class ViewPortListResource;
class FontResource;
class CMapResource;
class VInitCyclResource;

#define DECOMPRESS_SIZE 0x7000

typedef void (BoltFile::*BoltMethodPtr)();

class ResolveEntry {
public:
	uint32 _id;
	byte **_p;

	ResolveEntry(uint32 id, byte **p) { _id = id; _p = p; }
};

class BoltFilesState {
public:
	VoyeurEngine *_vm;
	BoltFile *_curLibPtr;
	BoltGroup *_curGroupPtr;
	BoltEntry *_curMemberPtr;
	byte *_curMemInfoPtr;
	int _fromGroupFlag;
	byte _xorMask;
	bool _encrypt;
	int _curFilePosition;
	int _bufferEnd;
	int _bufferBegin;
	int _bytesLeft;
	int _bufSize;
	byte *_bufStart;
	byte *_bufPos;
	byte _decompressBuf[DECOMPRESS_SIZE];
	int _historyIndex;
	byte _historyBuffer[0x200];
	int _runLength;
	int _decompState;
	int _runType;
	int _runValue;
	int _runOffset;
	Common::File *_curFd;
	Common::Array<ResolveEntry> _resolves;

	byte *_boltPageFrame;
	int _sImageShift;
	bool _SVGAReset;
public:
	BoltFilesState();

	byte *decompress(byte *buf, int size, int mode);
	void nextBlock();

	void EMSGetFrameAddr(byte **pageFrame) {} // TODO: Maybe?
	bool EMSAllocatePages(uint *planeSize) { return false; } // TODO: Maybe?
	void EMSMapPageHandle(int planeSize, int idx1, int idx2) {} // TODO: Maybe?
};

class BoltFile {
private:
	static const BoltMethodPtr _fnInitType[25];
private:
	BoltFilesState &_state;
	Common::Array<BoltGroup> _groups;
	Common::File _file;

	// initType method table
	void initDefault();
	void sInitPic();
	void vInitCMap();
	void vInitCycl();
	void initViewPort();
	void initViewPortList();
	void initFontInfo();
	void initSoundMap();
private:
	void resolveAll();
	byte *getBoltMember(uint32 id);

	void termType() {}	// TODO
	void initMem(int id) {}	// TODO
	void termMem() {}	// TODO
	void initGro() {}	// TODO
	void termGro() {}	// TODO
public:
	BoltFile(BoltFilesState &state);
	~BoltFile();

	bool getBoltGroup(uint32 id);
	void freeBoltGroup(uint32 id);
	byte *memberAddr(uint32 id);
	byte *memberAddrOffset(uint32 id);
	void resolveIt(uint32 id, byte **p);
	void resolveFunction(uint32 id, GraphicMethodPtr *fn);

	BoltEntry &getBoltEntry(uint32 id);
	PictureResource *getPictureResouce(uint32 id);
	CMapResource *getCMapResource(uint32 id);
};

class BoltGroup {
private:
	Common::SeekableReadStream *_file;
public:
	byte _loaded;
	bool _processed;
	bool _callInitGro;
	int _termGroIndex;
	int _count;
	int _fileOffset;
	Common::Array<BoltEntry> _entries;
public:
	BoltGroup(Common::SeekableReadStream *f); 

	void load();
};


class BoltEntry {
private:
	Common::SeekableReadStream *_file;
public:
	byte _mode;
	byte _field1;
	byte _initMethod;
	int _fileOffset;
	byte _xorMask;
	int _size;
	byte *_data;

	PictureResource *_picResource;
	ViewPortResource *_viewPortResource;
	ViewPortListResource *_viewPortListResource;
	FontResource *_fontResource;
	CMapResource *_cMapResource;
	VInitCyclResource *_vInitCyclResource;
public:
	BoltEntry(Common::SeekableReadStream *f);
	virtual ~BoltEntry();

	void load();
	bool hasResource() const;
};

class FilesManager {
private:
	int _decompressSize;
public:
	BoltFilesState _boltFilesState;
	BoltFile *_curLibPtr;
public:
	FilesManager();
	void setVm(VoyeurEngine *vm) { _boltFilesState._vm = vm; }

	bool openBoltLib(const Common::String &filename, BoltFile *&boltFile);
};

class DisplayResource {
public:
	uint16 _flags;
};

class PictureResource: public DisplayResource {
public:
	byte _select;
	byte _pick;
	byte _onOff;
	byte _depth;
	Common::Rect _bounds;
	uint32 _maskData;
	uint _planeSize;

	byte *_imgData;
public:
	PictureResource(BoltFilesState &state, const byte *src);
	virtual ~PictureResource();
};

typedef void (ViewPortResource::*ViewPortMethodPtr)();

class ViewPortResource: public DisplayResource {
private:
	BoltFilesState &_state;
private:
	void setupViewPort(PictureResource *page, Common::Rect *clipRect, ViewPortSetupPtr setupFn,
		ViewPortAddPtr addFn, ViewPortRestorePtr restoreFn);
public:
	ViewPortResource *_next;
	int _pageCount;
	int _pageIndex;
	int _lastPage;
	Common::Rect _bounds;
	int _field18;
	PictureResource *_currentPic;
	PictureResource *_activePage;
	PictureResource *_pages[2];
	byte *_field30;

	// Rect lists and counts. Note that _rectListCount values of '-1' seem to have
	// special significance, which is why I'm not making them redundant in favour 
	// of the arrays' .size() method
	Common::Array<Common::Rect> *_rectListPtr[3];
	int _rectListCount[3];

	Common::Rect _clipRect;
	byte *_field7A;
	GraphicMethodPtr _fn1;
	ViewPortSetupPtr _setupFn;
	ViewPortAddPtr _addFn;
	ViewPortRestorePtr _restoreFn;
public:
	ViewPortResource(BoltFilesState &state, const byte *src);
	virtual ~ViewPortResource();

	void setupViewPort();
};

class ViewPortPalEntry  {
public:
	uint16 _rEntry, _gEntry, _bEntry;
	uint16 _rChange, _gChange, _bChange;
	uint16 _palIndex;
public:
	ViewPortPalEntry(const byte *src);
};

class ViewPortListResource {
public:
	Common::Array<ViewPortPalEntry> _palette;
	Common::Array<ViewPortResource *> _entries;

	ViewPortListResource(BoltFilesState &state, const byte *src);
	virtual ~ViewPortListResource() {}
};

class FontResource {
public:
	byte *_fieldC;

	FontResource(BoltFilesState &state, const byte *src);
	virtual ~FontResource() {}
};

class CMapResource {
private:
	VoyeurEngine *_vm;
public:
	int _steps;
	int _fadeStatus;
	int _start;
	int _end;
	byte *_entries;
public:
	CMapResource(BoltFilesState &state, const byte *src);
	virtual ~CMapResource();

	void startFade();
};

class VInitCyclResource {
public:
	byte *_ptr[4];
public:
	VInitCyclResource(BoltFilesState &state, const byte *src);
	virtual ~VInitCyclResource() {}
};

} // End of namespace Voyeur

#endif /* VOYEUR_FILES_H */
