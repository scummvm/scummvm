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
 */

#ifndef VOYEUR_FILES_H
#define VOYEUR_FILES_H

#include "common/scummsys.h"
#include "common/file.h"
#include "common/rect.h"
#include "common/str.h"
#include "voyeur/screen.h"

namespace Voyeur {

class VoyeurEngine;
class BoltFile;
class BoltGroup;
class BoltEntry;
class RectResource;
class PictureResource;
class ViewPortResource;
class ViewPortListResource;
class FontResource;
class CMapResource;
class VInitCycleResource;
class PtrResource;
class ControlResource;
class StateResource;
class ThreadResource;

#define DECOMPRESS_SIZE 0x7000

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
	bool _decompState;
	int _runType;
	int _runValue;
	int _runOffset;
	Common::File *_curFd;
	Common::Array<ResolveEntry> _resolves;

	byte *_boltPageFrame;
public:
	BoltFilesState(VoyeurEngine *vm);

	byte *decompress(byte *buf, int size, int mode);
	void nextBlock();
};

class BoltFile {
private:
	Common::Array<BoltGroup> _groups;
protected:
	BoltFilesState &_state;

	virtual void initResource(int resType) = 0;
	void initDefault();
private:
	void resolveAll();
	byte *getBoltMember(uint32 id);

public:
	Common::File _file;

	BoltFile(const Common::String &filename, BoltFilesState &state);
	virtual ~BoltFile();

	BoltGroup *getBoltGroup(uint16 id);
	void freeBoltGroup(uint16 id);
	void freeBoltMember(uint32 id);
	byte *memberAddr(uint32 id);
	byte *memberAddrOffset(uint32 id);
	void resolveIt(uint32 id, byte **p);
	void resolveFunction(uint32 id, ScreenMethodPtr *fn);

	BoltEntry &boltEntry(uint16 id);
	BoltEntry &getBoltEntryFromLong(uint32 id);
	PictureResource *getPictureResource(uint32 id);
	CMapResource *getCMapResource(uint32 id);
};

class BVoyBoltFile: public BoltFile {
private:
	// initType method table
	void sInitRect();
	void sInitPic();
	void vInitCMap();
	void vInitCycl();
	void initViewPort();
	void initViewPortList();
	void initFontInfo();
	void initFont();
	void initSoundMap();
protected:
	void initResource(int resType) override;
public:
	BVoyBoltFile(BoltFilesState &state);
};

class StampBoltFile: public BoltFile {
private:
	void initThread();
	void initState();
	void initPtr();
	void initControl();
protected:
	void initResource(int resType) override;
public:
	StampBoltFile(BoltFilesState &state);
};

class BoltGroup {
private:
	Common::SeekableReadStream *_file;
public:
	bool _loaded;
	bool _processed;
	int _count;
	int _fileOffset;
	Common::Array<BoltEntry> _entries;
public:
	BoltGroup(Common::SeekableReadStream *f);
	virtual ~BoltGroup();

	void load(uint16 groupId);
	void unload();
};


class BoltEntry {
private:
	Common::SeekableReadStream *_file;
public:
	uint16 _id;
	byte _mode;
	byte _initMethod;
	int _fileOffset;
	int _size;
	byte *_data;

	// bvoy.blt resource types
	RectResource *_rectResource;
	PictureResource *_picResource;
	ViewPortResource *_viewPortResource;
	ViewPortListResource *_viewPortListResource;
	FontResource *_fontResource;
	FontInfoResource *_fontInfoResource;
	CMapResource *_cMapResource;
	VInitCycleResource *_vInitCycleResource;

	// stampblt.blt resource types
	PtrResource *_ptrResource;
	ControlResource *_controlResource;
	StateResource *_stateResource;
	ThreadResource *_threadResource;
public:
	BoltEntry(Common::SeekableReadStream *f, uint16 id);
	virtual ~BoltEntry();

	void load();
	bool hasResource() const;
};

class FilesManager {
public:
	BoltFilesState *_boltFilesState;
	BoltFile *_curLibPtr;
public:
	FilesManager(VoyeurEngine *vm);
	~FilesManager();

	bool openBoltLib(const Common::String &filename, BoltFile *&boltFile);
	byte *fload(const Common::String &filename, int *size);
};

class RectEntry: public Common::Rect {
public:
	int _arrIndex;
	int _count;

	RectEntry(int x1, int y1, int x2, int y2, int arrIndex, int count);
};

class RectResource: public Common::Rect {
public:
	Common::Array<RectEntry> _entries;
public:
	RectResource(const byte *src, int size, bool isExtendedRects);
	RectResource(int xp, int yp, int width, int height);
	virtual ~RectResource() {}
};

/* bvoy.blt resource types */

enum PictureFlag {
	PICFLAG_2 = 2, PICFLAG_PIC_OFFSET = 8, PICFLAG_CLEAR_SCREEN = 0x10,
	PICFLAG_20 = 0x20, PICFLAG_HFLIP = 0x40, PICFLAG_VFLIP = 0x80, PICFLAG_100 = 0x100,
	PICFLAG_CLEAR_SCREEN00 = 0x1000
};

enum DisplayFlag {
	DISPFLAG_1 = 1, DISPFLAG_2 = 2, DISPFLAG_4 = 4, DISPFLAG_8 = 8,
	DISPFLAG_10 = 0x10, DISPFLAG_20 = 0x20, DISPFLAG_40 = 0x40, DISPFLAG_80 = 0x80,
	DISPFLAG_100 = 0x100, DISPFLAG_200 = 0x200, DISPFLAG_400 = 0x400,
	DISPFLAG_800 = 0x800, DISPFLAG_1000 = 0x1000, DISPFLAG_2000 = 0x2000,
	DISPFLAG_4000 = 0x4000, DISPFLAG_VIEWPORT = 0x8000, DISPFLAG_CURSOR = 0x10000,
	DISPFLAG_NONE = 0};

class DisplayResource {
private:
	VoyeurEngine *_vm;
public:
	uint32 _flags;
public:
	DisplayResource();
	DisplayResource(VoyeurEngine *vm);

	/**
	 * Fill a box of the given size at the current _drawPtr location
	 */
	void sFillBox(int width, int height);

	/**
	 * Draw text at the current pen position
	 */
	int drawText(const Common::String &msg);

	/**
	 * Return the width of a given text in the current font
	 */
	int textWidth(const Common::String &msg);

	/**
	 * Clip the given rectangle by the currently viewable area
	 */
	bool clipRect(Common::Rect &rect);
};

class PictureResource: public DisplayResource {
private:
	/**
	 * Flip the image data horizontally
	 */
	void flipHorizontal(const byte *data);

	/**
	 * Flip the image data vertically
	 */
	void flipVertical(const byte *data);
public:
	byte _select;
	byte _pick;
	byte _onOff;
	Common::Rect _bounds;
	uint32 _maskData;
	uint _planeSize;
	byte _keyColor;

	/**
	 * Image data for the picture
	 */
	byte *_imgData;

	/**
	 * Flag to indicate whether to free the image data
	 */
	DisposeAfterUse::Flag _freeImgData;
public:
	PictureResource(BoltFilesState &state, const byte *src);
	PictureResource(int flags, int select, int pick, int onOff,
		const Common::Rect &bounds, int maskData, byte *imgData, int planeSize);
	PictureResource(Graphics::Surface *surface);
	PictureResource();
	virtual ~PictureResource();
};

typedef void (ViewPortResource::*ViewPortMethodPtr)();

class ViewPortResource: public DisplayResource {
private:
	BoltFilesState &_state;
private:
	void setupViewPort(PictureResource *page, Common::Rect *clippingRect, ViewPortSetupPtr setupFn,
		ViewPortAddPtr addFn, ViewPortRestorePtr restoreFn);
public:
	ViewPortResource *_parent;
	ViewPortSetupPtr _setupFn;
	int _pageCount;
	ViewPortAddPtr _addFn;
	int _pageIndex;
	ViewPortRestorePtr _restoreFn;
	int _lastPage;
	ScreenMethodPtr _fn1;
	Common::Rect _bounds;
	PictureResource *_currentPic;
	PictureResource *_activePage;
	PictureResource *_pages[2];

	// Rect lists and counts. Note that _rectListCount values of '-1' seem to have
	// special significance, which is why I'm not making them redundant in favor
	// of the arrays' .size() method
	Common::Array<Common::Rect> *_rectListPtr[3];
	int _rectListCount[3];

	Common::Rect _clipRect;
	Common::Rect _fontRect;
public:
	ViewPortResource(BoltFilesState &state, const byte *src);
	virtual ~ViewPortResource();

	void setupViewPort();
	void setupViewPort(PictureResource *pic, Common::Rect *clippingRect = NULL);
	void addSaveRect(int pageIndex, const Common::Rect &r);
	void fillPic(byte onOff);
	void drawIfaceTime();
	void drawPicPerm(PictureResource *pic, const Common::Point &pt);
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
	int _palIndex;

	ViewPortListResource(BoltFilesState &state, const byte *src);
	virtual ~ViewPortListResource() {}
};

class FontResource {
public:
	int _minChar, _maxChar;
	int _fontDepth;
	int _padding;
	int _fontHeight;
	int _topPadding;
	int *_charWidth;
	byte *_charOffsets;
	byte *_charImages;

	FontResource(BoltFilesState &state, byte *src);
	virtual ~FontResource();
};

enum FontJustify { ALIGN_LEFT = 0, ALIGN_CENTER = 1, ALIGN_RIGHT = 2 };

class FontInfoResource {
public:
	FontResource *_curFont;
	byte _picFlags;
	byte _picSelect;
	byte _picPick;
	byte _picOnOff;
	byte _fontFlags;
	FontJustify _justify;
	int _fontSaveBack;
	Common::Point _pos;
	int _justifyWidth;
	int _justifyHeight;
	Common::Point _shadow;
	int _foreColor;
	int _backColor;
	int _shadowColor;
public:
	FontInfoResource(BoltFilesState &state, const byte *src);
	FontInfoResource();
	FontInfoResource(byte picFlags, byte picSelect, byte picPick, byte picOnOff, byte fontFlags,
		FontJustify justify, int fontSaveBack, const Common::Point &pos, int justifyWidth,
		int justifyHeight, const Common::Point &shadow, int foreColor, int backColor,
		int shadowColor);
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

class VInitCycleResource {
private:
	BoltFilesState &_state;
public:
	int _type[4];
	byte *_ptr[4];
public:
	VInitCycleResource(BoltFilesState &state, const byte *src);
	virtual ~VInitCycleResource() {}

	void vStartCycle();
	void vStopCycle();
};

/* stampblt.blt resources */

class PtrResource {
public:
	Common::Array<BoltEntry *> _entries;

	PtrResource(BoltFilesState &state, const byte *src);
	virtual ~PtrResource() {}
};

class ControlResource {
public:
	int _memberIds[8];
	byte *_entries[8];
	int _stateId;
	StateResource *_state;

	ControlResource(BoltFilesState &state, const byte *src);
	virtual ~ControlResource() {}
};

/**
 * Stores data about the intended victim
 */
class StateResource {
public:
	int _vals[4];
	int &_victimIndex;
	int &_victimEvidenceIndex;
	int &_victimMurderIndex;

	StateResource(BoltFilesState &state, const byte *src);
	virtual ~StateResource() {}

	/**
	 * Synchronizes the game data
	 */
	void synchronize(Common::Serializer &s);
};

class ThreadResource {
public:
	static int _useCount[8];
	static void initUseCount();
	static void unloadAllStacks(VoyeurEngine *vm);

	static void init();
private:
	VoyeurEngine *_vm;
	Common::Point _aptPos;
private:
	bool getStateInfo();
	byte *getDataOffset();
	void getButtonsText();
	void getButtonsFlags();
	void performOpenCard();
	const byte *getRecordOffset(const byte *p);
	const byte *getNextRecord(const byte *p);
	const byte *getSTAMPCard(int cardId);
	int getStateFromID(uint32 id);
	uint32 getSID(int sid);
	void cardAction(const byte *p);
	void doSTAMPCardAction();
	bool goToStateID(int stackId, int id);
	const byte *cardPerform(const byte *card);
	bool cardPerform2(const byte *p, int cardCmdId);
	void savePrevious();
	void setButtonFlag(int idx, byte bits);
	void clearButtonFlag(int idx, byte bits);

	/**
	 * Frees the apartment screen data
	 */
	void freeTheApt();

	/**
	 * Does any necessary animation at the start or end of showing the apartment.
	 */
	void doAptAnim(int mode);

	/**
	 * Updates the mansion scroll position if ncessary, and returns true if it
	 * has been changed.
	 */
	bool checkMansionScroll();
public:
	int _stateId;
	int _stackId;
	int _savedStateId;
	int _savedStackId;
	int _newStateId;
	int _newStackId;
	int _stateFlags;
	int _stateCount;
	int _parseCount;
	uint32 _nextStateId;
	byte *_threadInfoPtr;
	byte _buttonFlags[64];
	byte _buttonIds[64];
	byte *_ctlPtr;
	byte *_playCommandsPtr;

	/**
	 * Loads the specified stack
	 */
	bool loadAStack(int stackId);

	/**
	 * Unloads the specified stack
	 */
	void unloadAStack(int stackId);

	/**
	 * Initializes data for the thread based on the current state
	 */
	bool doState();

public:
	ThreadResource(BoltFilesState &state, const byte *src);
	virtual ~ThreadResource() {}

	/**
	 * Initialize the thread
	 */
	void initThreadStruct(int idx, int id);

	/**
	 * Go to a new state and/or stack
	 */
	bool goToState(int stackId, int stateId);

	bool chooseSTAMPButton(int buttonId);

	/**
	 * Parses the script commands from the currently active stack
	 */
	void parsePlayCommands();

	/**
	 * Do the camera view looking at the mansion
	 */
	int doInterface();

	/**
	 * Do the display of a room that has one or more evidence hotspots
	 * available for display
	 */
	void doRoom();

	/**
	 * Shows the apartment screen
	 */
	int doApt();

	/**
	 * Loads data needed for displaying the initial apartment screen
	 */
	void loadTheApt();

	/**
	 * Synchronizes the game data
	 */
	void synchronize(Common::Serializer &s);
};

} // End of namespace Voyeur

#endif /* VOYEUR_FILES_H */
