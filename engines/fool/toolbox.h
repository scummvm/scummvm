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

#ifndef FOOL_TOOLBOX_H
#define FOOL_TOOLBOX_H

#include "common/bitstream.h"
#include "common/hashmap.h"
#include "common/system.h"
#include "common/macresman.h"
#include "common/str.h"
#include "common/ustr.h"

#include "graphics/cursor.h"
#include "image/pict.h"
#include "graphics/framelimiter.h"
#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/macwindow.h"
#include "graphics/macgui/macwindowmanager.h"

#include "fool/siphash/halfsip.h"

namespace Fool {

struct RGBColor {
	uint16 red;
	uint16 green;
	uint16 blue;
};

struct ColorSpec {
	uint16 value;
	RGBColor rgb;
};

struct ColorTable {
	uint32 ctSeed;
	uint16 ctFlags;
	uint16 ctSize;
	Common::Array<ColorSpec> ctTable;
};

struct Region {
	uint16 rgnSize;
	Common::Rect rgnBBox;
	Common::Array<int16> rgnData;
};

struct PixMap {
	uint32 baseAddr;
	uint16 rowBytes;
	Common::Rect bounds;
	uint16 pmVersion;
	uint16 packType;
	uint32 packSize;
	uint32 hRes;
	uint32 vRes;
	uint16 pixelType;
	uint16 pixelSize;
	uint16 cmpCount;
	uint16 cmpSize;
	uint32 planeBytes;
	uint32 pmTable;
	uint32 pmReserved;
	bool _isBitMap = false;
};

struct Polygon {
	uint16 polySize;
	Common::Rect polyBBox;
	Common::Array<Common::Point> polyPoints;
};

// In QuickDraw 0 means white and 1 means black
struct Pattern {
	uint8 data[8];

	Common::String format() const {
		return Common::String::format("%02x%02x%02x%02x%02x%02x%02x%02x", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
	}
};

enum PictureOpType : uint16 {
	kOpNOP =		0x0000,
	kOpClip =		0x0001,
	kOpBkPat =		0x0002,
	kOpTxFont =		0x0003,
	kOpTxFace =		0x0004,
	kOpTxMode =		0x0005,
	kOpSpExtra =	0x0006,
	kOpPnSize =		0x0007,
	kOpPnMode =		0x0008,
	kOpPnPat =		0x0009,
	kOpFillPat =	0x000a,
	kOpOvSize =		0x000b,
	kOpOrigin =		0x000c,
	kOpTxSize =		0x000d,
	kOpFgColor =	0x000e,
	kOpBkColor =	0x000f,
	kOpTxRatio =	0x0010,
	kOpVersion2 =	0x0011,
	kOpBkPixPat	=	0x0012,
	kOpPnPixPat =	0x0013,
	kOpFillPixPat = 0x0014,
	kOpPnLocHFrac = 0x0015,
	kOpChExtra =	0x0016,
	// reserved 0x0017
	// reserved 0x0018
	// reserved 0x0019
	kOpRGBFgCol =	0x001a,
	kOpRGBBkCol =	0x001b,
	kOpHiliteMode = 0x001c,
	kOpHiliteColor = 0x001d,
	kOpDefHilite =	0x001e,
	kOpColor =		0x001f,
	kOpLine =		0x0020,
	kOpLineFrom =	0x0021,
	kOpShortLine =	0x0022,
	kOpShortLineFrom = 0x0023,
	// reserved 0x0024
	// reserved 0x0025
	// reserved 0x0026
	// reserved 0x0027
	kOpLongText	=	0x0028,
	kOpDHText =		0x0029,
	kOpDVText =		0x002a,
	kOpDHDVText =	0x002b,
	kOpFontName =	0x002c,
	kOpLineJustify = 0x002d,
	kOpGlyphState = 0x002e,
	// reserved 0x002f
	kOpFrameRect = 0x0030,
	kOpPaintRect = 0x0031,
	kOpEraseRect = 0x0032,
	kOpInvertRect = 0x0033,
	kOpFillRect = 0x0034,
	// reserved 0x0035
	// reserved 0x0036
	// reserved 0x0037
	kOpFrameSameRect = 0x0038,
	kOpPaintSameRect = 0x0039,
	kOpEraseSameRect = 0x003a,
	kOpInvertSameRect = 0x003b,
	kOpFillSameRect = 0x003c,
	// reserved 0x003d
	// reserved 0x003e
	// reserved 0x003f
	kOpFrameRRect = 0x0040,
	kOpPaintRRect = 0x0041,
	kOpEraseRRect = 0x0042,
	kOpInvertRRect = 0x0043,
	kOpFillRRect = 0x0044,
	// reserved 0x0045
	// reserved 0x0046
	// reserved 0x0047
	kOpFrameSameRRect = 0x0048,
	kOpPaintSameRRect = 0x0049,
	kOpEraseSameRRect = 0x004a,
	kOpInvertSameRRect = 0x004b,
	kOpFillSameRRect = 0x004c,
	// reserved 0x004d
	// reserved 0x004e
	// reserved 0x004f
	kOpFrameOval = 0x0050,
	kOpPaintOval = 0x0051,
	kOpEraseOval = 0x0052,
	kOpInvertOval = 0x0053,
	kOpFillOval = 0x0054,
	// reserved 0x0055
	// reserved 0x0056
	// reserved 0x0057
	kOpFrameSameOval = 0x0058,
	kOpPaintSameOval = 0x0059,
	kOpEraseSameOval = 0x005a,
	kOpInvertSameOval = 0x005b,
	kOpFillSameOval = 0x005c,
	// reserved 0x005d
	// reserved 0x005e
	// reserved 0x005f

	kOpFramePoly = 0x0070,
	kOpPaintPoly = 0x0071,
	kOpErasePoly = 0x0072,
	kOpInvertPoly = 0x0073,
	kOpFillPoly = 0x0074,
	// reserved 0x0075
	// reserved 0x0076
	// reserved 0x0077
	kOpFrameSamePoly = 0x0078,
	kOpPaintSamePoly = 0x0079,
	kOpEraseSamePoly = 0x007a,
	kOpInvertSamePoly = 0x007b,
	kOpFillSamePoly = 0x007c,
	// reserved 0x007d
	// reserved 0x007e
	// reserved 0x007f

	kOpBitsRect = 0x0090,
	kOpBitsRgn = 0x0091,

	kOpPackBitsRect = 0x0098,
	kOpPackBitsRgn = 0x0099,

	kOpShortComment = 0x00a0,
	kOpLongComment = 0x00a1,

	kOpEndPic = 0x00ff,

	kOpHeaderOp = 0x0c00,
	kOpVersion1 = 0x1101,
};

// BitMap is the monochrome surface format.
typedef Common::SharedPtr<Graphics::ManagedSurface> BitMap;
typedef Common::SharedPtr<Polygon> PolyHandle;
typedef Common::SharedPtr<Common::Array<byte>> Handle;
typedef Common::SharedPtr<Region> RgnHandle;

struct Picture {
	uint32 picSize = 0;
	Common::Rect picFrame;
	Common::Array<byte> picData;
	size_t picPtr = 0;
	// simulate slow drawing
	uint32 _opsPerTick = 0;

	void pushOp(PictureOpType op) {
		picData.push_back(op >> 8);
		picData.push_back(op & 0xff);
	}

	void pushOpU16(PictureOpType op, uint16 val) {
		picData.push_back(op >> 8);
		picData.push_back(op & 0xff);
		picData.push_back(val >> 8);
		picData.push_back(val & 0xff);
	}

	void pushHeader();
	void pushOpPoly(PictureOpType op, const PolyHandle &poly);
	void pushOpPat(PictureOpType op, const Pattern &pat);
	void pushOpRect(PictureOpType op, const Common::Rect &rect);
	void pushOpPoint(PictureOpType op, const Common::Point &point);
	void pushOpPointStr(PictureOpType op, const Common::Point &point, const Common::String &str);
};


typedef Common::SharedPtr<Picture> PicHandle;

typedef uint32 ResType;
typedef size_t Size;

};

template<>
struct Common::Hash<Fool::Handle> {
	uint operator()(const Fool::Handle& h) const {
		void *target = h.get();
		uint result;
		Fool::halfsiphash((const void *)&target, sizeof(intptr_t), "TBHANDLE", (byte *)&result, sizeof(uint));
		return result;
	}
};

template<>
struct Common::Hash<Fool::PicHandle> {
	uint operator()(const Fool::PicHandle& h) const {
		void *target = h.get();
		uint result;
		Fool::halfsiphash((const void *)&target, sizeof(intptr_t), "TBHANDLE", (byte *)&result, sizeof(uint));
		return result;
	}
};

namespace Fool {

// TYPE BOOLEAN: int16
// TYPE INTEGER: int16
// TYPE LONGINT: int32
// TYPE CHAR: int16
// TYPE Rect: top: INTEGER; left: INTEGER; bottom: INTEGER; right: INTEGER;
// TYPE Point: v: INTEGER; h: INTEGER;

class Cursor: public Graphics::Cursor {
public:
	uint16 data[16];
	uint16 mask[16];
	Common::Point mouse;

	void render() {
		for (int y = 0; y < 16; y++) {
			for (int x = 0; x < 16; x++) {
				this->_surface[y*16+x] = (this->data[y] & (1 << x)) ? 1 : 0;
				this->_mask[y*16+x] = (this->mask[y] & (1 << x)) ? kCursorMaskOpaque : kCursorMaskTransparent;
			}
		}
	}

	virtual uint16 getWidth() const { return 16; }
	virtual uint16 getHeight() const { return 16; }
	virtual uint16 getHotspotX() const { return mouse.x; }
	virtual uint16 getHotspotY() const { return mouse.y; }
	virtual byte getKeyColor() const { return 3; }
	virtual const byte *getSurface() const { return _surface; }
	virtual const byte *getMask() const { return _mask; }
	virtual const byte *getPalette() const { return _palette; }
	virtual byte getPaletteStartIndex() const { return 0; }
	virtual uint16 getPaletteCount() const { return _paletteCount; }


private:
	byte _surface[16*16] = { 0 };
	byte _mask[16*16] = { 0 };
	byte _palette[6] = { 0xff, 0xff, 0xff, 0x00, 0x00, 0x00 };
	uint16 _paletteCount = 2;
}; // 0x44

typedef uint32 OSType;
typedef uint16 OSErr;
typedef uint32 ProcPtr;

enum EventCode : uint16 {
	kNullEvent = 0,
	kMouseDown = 1,
	kMouseUp = 2,
	kKeyDown = 3,
	kKeyUp = 4,
	kAutoKey = 5,
	kUpdateEvt = 6,
	kDiskEvt = 7,
	kActivateEvt = 8,
	kNetworkEvt = 10,
	kDriverEvt = 11,
	kApp1Evt = 12,
	kApp2Evt = 13,
	kApp3Evt = 14,
	kOsEvt = 15,
	kHighLevelEvt = 23,
	kScummVMQuitEvt = 24,
	kScummVMReturnToLauncherEvt = 25,
};

enum ModifierMask : uint16 {
	kModForegroundSwitch = 0x0001,
	kModMouseButtonUp = 0x0080,
	kModCommandKeyDown = 0x0100,
	kModLShiftKeyDown = 0x0200,
	kModCapsLockKeyDown = 0x0400,
	kModLOptionKeyDown = 0x0800,
	kModLControlKeyDown = 0x1000,
	kModRShiftKeyDown = 0x2000,
	kModROptionKeyDown = 0x4000,
	kModRControlKeyDown = 0x8000,
};

enum SourceMode : uint16 {
	kSrcCopy = 0,
	kSrcOr = 1,
	kSrcXor = 2,
	kSrcBic = 3,
	kNotSrcCopy = 4,
	kNotSrcOr = 5,
	kNotSrcXor = 6,
	kNotSrcBic = 7
};

enum PatternMode : uint16 {
	kPatCopy = 8,
	kPatOr = 9,
	kPatXor = 10,
	kPatBic = 11,
	kNotPatCopy = 12,
	kNotPatOr = 13,
	kNotPatXor = 14,
	kNotPatBic = 15
};

enum WindowDefinition {
	kDocumentProc = 0,
	kDBoxProc = 1,
	kPlainDBox = 2,
	kAltDBoxProc = 3,
	kNoGrowDocProc = 4,
	kRDocProc = 16,
};

// for text style: use kMacFont*

struct GrafPort {
	uint16 device = 0;
	BitMap portBits;
	Common::Rect portRect;
	RgnHandle visRgn;
	RgnHandle clipRgn;
	Pattern bkPat = { { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } }; // white
	Pattern fillPat = { { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } }; // black
	Common::Point pnLoc = {0, 0};
	Common::Point pnSize = {1, 1};
	PatternMode pnMode = kPatCopy;
	Pattern pnPat = { { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } }; // black
	uint16 pnVis = 0;
	uint16 txFont = 0;
	uint16 txFace;
	SourceMode txMode = kSrcOr;
	uint16 txSize = 0;
	uint32 spExtra = 0;
	uint32 fgColor;
	uint32 bkColor;
	uint16 colrBit = 0;
	uint16 patStretch = 0;
	PicHandle picSave;
	RgnHandle rgnSave;
	PolyHandle polySave;

};
typedef GrafPort * GrafPtr;

struct WindowRecord {
	GrafPort port;
	uint16 windowKind;
};

struct EventRecord {
	EventCode what = kNullEvent; // 0
	uint32 message = 0; // 2
	WindowRecord *windowPtr = nullptr; // also 2
	uint32 when = 0;  // 6
	Common::Point where; // 10
	uint16 modifiers = 0;  // 14
};

struct ToolboxResInfo {
	int16 fileID;
	ResType type;
	uint16 resID;
	Handle handle;
	Common::String name;
};

struct FontInfo {
	int16 ascent;
	int16 descent;
	int16 widMax;
	int16 leading;
};

struct MenuInfo {
	uint16 menuID;
	uint16 menuWidth;
	uint16 menuHeight;
	Handle menuProc;
	uint32 enableFlags;
	// original record stores this as a single string blob, we split it up.
	Common::Array<Common::U32String> menuData;
};

typedef Common::SharedPtr<MenuInfo> MenuHandle;

struct ParamBlockRec {
	uint32 qLink = 0;
	uint16 qType = 0;
	uint16 ioTrap = 0;
	uint32 ioCmdAddr = 0;
	uint32 ioCompletion = 0;
	OSErr ioResult = 0;
	Common::String ioNamePtr;
	uint16 ioVRefNum = 0;
};

struct SFTypeList {
	OSType types[4];
};

struct SFReply {
	bool good; // 0
	bool copy; // 1
	OSType fType; // 2
	int16 vRefNum; // 6
	int16 version; // 8
	Common::String fName; // 10
}; // total: 76

enum SynthMode : int16 {
	swMode = -1,	// square-wave
	ftMode = 1,		// four-tone
	ffMode = 0,		// free-form
};

struct Tone {
	uint16 count;
	uint16 amplitude;
	uint16 duration;
};

struct SynthRec {
	SynthMode mode;
};
typedef SynthRec * SynthPtr;

struct SWSynthRec : SynthRec {
	Common::Array<Tone> triplets;
};

class Toolbox {

public:
	// Compatibility shim for the Macintosh Toolbox/QuickDraw API.
	Toolbox();
	~Toolbox();

	Graphics::MacWindow *_defaultWindow;
	Graphics::MacMenu *_defaultMenu;
	BitMap _defaultBits;


	// toolbox.cpp

	// PROCEDURE Delay (numTicks: LONGINT; VAR finalTicks: LONGINT);
	// Delay causes the system to wait for the number of ticks (sixtieths of a second) specified by numTicks, and returns in finalTicks the total number of ticks from system startup to the end of the delay.
	uint32 Delay(uint32 numTicks);

	// PROCEDURE FlushEvents (eventMask, stopMask: INTEGER);
	// FlushEvents removes events from the event queue as specified by the given event masks. It
	// removes all events of the type or types specified by eventMask, up to but not including the first
	// event of any type specified by stopMask; if the event queue doesn't contain any events of the
	// types specified by eventMask, it does nothing. To remove all events specified by eventMask, use
	// a stopMask value of 0.
	void FlushEvents(uint32 eventMask, uint32 stopMask);

	// FUNCTION GetNextEvent (eventMask: INTEGER; VAR theEvent: EventRecord) : BOOLEAN;
	// GetNextEvent returns the next available event of a specified type or types and, if the event is in
	// the event queue, removes it from the queue. The event is returned in the parameter theEvent. The
	// eventMask parameter specifies which event types are of interest. GetNextEvent returns the next
	// available event of any type designated by the mask, subject to the priority rules discussed above
	// under "Priority of Events". If no event of any of the designated types is available, GetNextEvent
	// returns a null event.
	bool GetNextEvent(uint32 eventMask, EventRecord &theEvent);

	// FUNCTION TickCount : LONGINT;
	// TickCount returns the current number of ticks (sixtieths of a second) since the system last started
	// up.
	uint32 TickCount();

	// toolbox_fileman.cpp
	OSErr PBGetVol(ParamBlockRec &paramBlock);

	OSErr PBSetVol(ParamBlockRec &paramBlock);

	void SFGetFile(const Common::Point &where, const Common::U32String &prompt, ProcPtr fileFilter, int16 numTypes, const SFTypeList &typeList, const ProcPtr &dlgHook, SFReply &reply);

	// PROCEDURE SFPutFile (where: Point; prompt: Str255; origName:
	// Str255; dlgHook: ProcPtr; VAR reply: SFReply);
	void SFPutFile(const Common::Point &where, const Common::U32String &prompt, const Common::U32String &origName, const ProcPtr &dlgHook, SFReply &reply);

	// toolbox_sound.cpp

	// PROCEDURE StartSound (synthRec: Ptr; numBytes: LONGINT; completionRtn: ProcPtr);
	// StartSound begins producing the sound described by the synthesizer buffer pointed to by
	// synthRec. NumBytes indicates the size of the synthesizer buffer (in bytes), and completionRtn
	// points to a completion routine to be executed when the sound finishes:
	// • If completionRtn is POINTER(-l), the sound will be produced synchronously.
	// • If completionRtn is NIL, the sound will be produced asynchronously, but no completion
	// routine will be executed.
	// • Otherwise, the sound will be produced asynchronously and the routine pointed to by
	// completionRtn will be executed when the sound finishes.
	void StartSound(SynthPtr synthRec, uint32 numBytes, ProcPtr completionRtn);

	// toolbox_resman.cpp

	// FUNCTION CountResources (theType: ResType): Integer;
	// Given a resource type, the CountResources function reads the resource maps in
	// memory for all resource forks open to your application. It returns as its function result
	// the total number of resources of the given type in all resource forks open to your
	// application.
	uint16 CountResources(ResType theType);

	// FUNCTION CurResFile: INTEGER;
	// CurResFile returns the reference number of the current resource file. You can call it when the
	// application starts up to get the reference number of its resource file.
	int16 CurResFile();

	// PROCEDURE DetachResource (theResource: Handle);
	// Given a handle to a resource, DetachResource replaces the handle to that resource in the resource
	// map with NIL (see Figure 7 above). The given handle will no longer be recognized as a handle to
	// a resource; if the Resource Manager is subsequendy called to get the detached resource, a new
	// handle will be allocated.
	void DetachResource(Handle &h);
	void DetachResource(PicHandle &h);

	// PROCEDURE DisposHandle (h: Handle);
	// DisposHandle releases the memory occupied by the relocatable block whose handle is h.
	void DisposHandle(Handle &h);

	// FUNCTION GetHandleSize (h: Handle) : Size;
	// GetHandleSize returns the logical size, in bytes, of the relocatable block whose handle is h. In
	// case of an error, GetHandleSize returns 0.
	Size GetHandleSize(Handle &h);

	// FUNCTION GetIndResource (theType: ResType; index: Integer): Handle;
	// Given an index ranging from 1 to the number of resources of a given type returned by
	// CountResources (that is, the number of resources of that type in all resource forks open
	// to your application), the GetIndResource function returns a handle to a resource of the
	// given type. If you call GetIndResource repeatedly over the entire range of the index, it
	// returns handles to all resources of the given type in all resource forks open to your
	// application.
	Handle GetIndResource(ResType theType, uint16 index);

	// FUNCTION GetNamedResource (theType: ResType; name: Str255) : Handle;
	// GetNamedResource is the same as GetResource (above) except that you pass a resource name
	// instead of an ID number.
	Handle GetNamedResource(ResType theType, const Common::String &name);

	// PROCEDURE GetResInfo (theResource: Handle; VAR theID: INTEGER;
	//		VAR theType: ResType; VAR name: Str255);
	// Given a handle to a resource, GetResInfo returns the ID number, type, and name of the resource.
	// If the given handle isn't a handle to a resource, GetResInfo will do nothing and the ResError
	// function will return the result code resNotFound.
	void GetResInfo(Handle &theResource, uint16 &theID, ResType &theType, Common::String &name);

	// FUNCTION GetResource (theType: ResType; theID: INTEGER) : Handle;
	// GetResource returns a handle to the resource having the given type and ID number, reading
	// the resource data into memory if it's not already in memory and if you haven't called
	// SetResLoad(FALSE) (see the warning above for GetIndResource). If the resource data is already
	// in memory, GetResource just returns the handle to the resource.
	Handle GetResource(ResType theType, uint16 theID);

	// FUNCTION HomeResFile (theResource: Handle) : INTEGER;
	// Given a handle to a resource, HomeResFile returns the reference number of the resource file
	// containing that resource. If the given handle isn't a handle to a resource, HomeResFile will
	// return -1 and the ResError function will return the result code resNotFound.
	int32 HomeResFile(Handle &theResource);

	// FUNCTION NewHandle (logicalSize: Size) : Handle;
	// NewHandle attempts to allocate a new relocatable block of logicalSize bytes from the current heap
	// zone and then return a handle to it. The new block will be unlocked and unpurgeable. If
	// logicalSize bytes can't be allocated, NewHandle returns NIL.
	Handle NewHandle(Size logicalSize);

	// FUNCTION OpenResFile (fileName: Str255): INTEGER;
	// OpenResFile opens the resource file having the given name and makes it the current resource file.
	// It reads the resource map from the file into memory and returns a reference number for the file. It
	// also reads in every resource whose resPreload attribute is set. If the resource file is already open,
	// it doesn't make it the current resource file; it simply returns the reference number.
	// If the file can't be opened, OpenResFile will return -1 and the ResError function will return an
	// appropriate Operating System result code. For example, an error occurs if there's no resource file
	// with the given name.
	int16 OpenResFile(const Common::Path &filename);

	// PROCEDURE ReleaseResource (theResource: Handle);
	// Given a handle to a resource, ReleaseResource releases the memory occupied by the resource
	// data, if any, and replaces the handle to that resource in the resource map with NIL (see Figure 7).
	// The given handle will no longer be recognized as a handle to a resource; if the Resource Manager
	// is subsequendy called to get the released resource, a new handle will be allocated. Use this
	// procedure only after you're completely through with a resource.
	void ReleaseResource(Handle &handle);
	void ReleaseResource(PicHandle &handle);

	// FUNCTION SizeResource(theResource: Handle) : LONGINT;
	// Given a handle to a resource, SizeResource returns the size in bytes of the resource in the
	// resource file. If the given handle isn't a handle to a resource, SizeResource will return - 1 and the
	// ResError function will return the result code resNotFound. It's a good idea to call SizeResource
	// and ensure that sufficient space is available before reading a resource into memory.
	int32 SizeResource(Handle &theResource);

	// PROCEDURE UseResFile (refNum: INTEGER);
	// Given the reference number of a resource file, UseResFile sets the current resource file to that
	// file. If there's no resource file open with the given reference number, UseResFile will do nothing
	// and the ResError function will return the result code resFNotFound. A refNum of 0 represents
	// the system resource file.
	void UseResFile(int16 refNum);


	// toolbox_menu.cpp

	// PROCEDURE AppendMenu(theMenu: MenuHandle; data: Str255);
	// AppendMenu adds an item or items to the end of the given menu, which must previously have
	// been allocated by NewMenu or read from a resource file by GetMenu. The data string consists of
	// the text of the menu item; it may be blank but should not be the empty string. If it begins with a
	// hyphen (-), the item will be a dividing line across the width of the menu. As described in the
	// section "Creating a Menu in Your Program", meta-characters may be embedded in the data string.
	void AppendMenu(MenuHandle &theMenu, const Common::U32String &data);

	// PROCEDURE CheckItem (theMenu: MenuHandle; item: INTEGER, checked: BOOLEAN);
	// CheckItem places or removes a check mark at the left of the given menu item. After you call
	// CheckItem with checked=TRUE, a check mark will appear each subsequent time the menu is
	// pulled down. Calling CheckItem with checked=FALSE removes the check mark from the menu
	// item (or, if it's marked with a different character, removes that mark).
	void CheckItem(MenuHandle &theMenu, uint16 item, bool checked);

	// PROCEDURE ClearMenuBar;
	// Call ClearMenuBar to remove all menus from the menu list when you want to start afresh with all
	// new menus. Be sure to call DrawMenuBar to update the menu bar.
	void ClearMenuBar();

	// FUNCTION CountMItems (theMenu: MenuHandle): INTEGER;
	// CountMItems returns the number of menu items in the given menu.
	uint16 CountMItems(MenuHandle &theMenu);

	// PROCEDURE DeleteMenu(menuID: INTEGER);
	// DeleteMenu deletes a menu from the menu list. If there's no menu with the given menu ID in the
	// menu list, DeleteMenu has no effect. Be sure to call DrawMenuBar to update the menu bar; the
	// menu tides following the deleted menu will move over to fill the vacancy.
	void DeleteMenu(uint16 menuID);

	// PROCEDURE DisableItem(theMenu: MenuHandle; item: INTEGER);
	// Given a menu item number in the item parameter, Disableltem disables that menu item; given 0 in
	// the item parameter, it disables the entire menu.
	void DisableItem(MenuHandle &theMenu, uint16 item);

	// PROCEDURE DisposeMenu(theMenu: MenuHandle);
	// Call DisposeMenu to release the memory occupied by a menu that you allocated with NewMenu.
	// (For menus read from a resource file with GetMenu, use the Resource Manager procedure
	// ReleaseResource instead.) This is useful if you've created temporary menus that you no longer
	// need.
	void DisposeMenu(MenuHandle &theMenu);

	// PROCEDURE DrawMenuBar;
	// DrawMenuBar redraws the menu bar according to the menu list, incorporating any changes since
	// the last call to DrawMenuBar. This procedure should always be called after a sequence of
	// InsertMenu or DeleteMenu calls, and after ClearMenuBar, SetMenuBar, or any other routine that
	// changes the menu list.
	void DrawMenuBar();

	// PROCEDURE EnableItem(theMenu: MenuHandle; item: INTEGER);
	// Given a menu item number in the item parameter, EnableItem enables the item (which may have
	// been disabled with the Disableltem procedure, or with the "(" meta-character in the AppendMenu
	// string).
	void EnableItem(MenuHandle &theMenu, uint16 item);

	// FUNCTION GetMHandle(menuID: INTEGER): MenuHandle;
	// Given the menu ID of a menu currentiy installed in the menu list, GetMHandle returns a handle to
	// that menu; given any other menu ID, it returns NIL.
	MenuHandle GetMHandle(uint16 menuID);

	// PROCEDURE HiliteMenu(menuID: INTEGER);
	// HiliteMenu highlights the title of the given menu, or does nothing if the tide is already
	// highlighted. Since only one menu tide can be highlighted at a time, it unhighlights any previously
	// highlighted menu tide. If menuID is 0 (or isn't the ID of any menu in the menu list), HiliteMenu
	// simply unhighlights whichever menu title is highlighted (if any).
	void HiliteMenu(uint16 menuID);

	// FUNCTION MenuSelect(startPt: Point): LONGINT;
	// When there's a mouse-down event in the menu bar, the application should call MenuSelect with
	// startPt equal to the point (in global coordinates) where the mouse button was pressed.
	// MenuSelect keeps control until the mouse button is released, tracking the mouse, pulling down
	// menus as needed, and highlighting enabled menu items under the cursor. When the mouse button
	// is released over an enabled item in an application menu, MenuSelect returns a long integer whose
	// high-order word is the menu ED of the menu, and whose low-order word is the menu item
	// number for the item chosen (see Figure 3). It leaves the selected menu tide highlighted. After
	// performing the chosen task, your application should call HiliteMenu(O) to remove the
	// highlighting from the menu title.
	uint32 MenuSelect(const Common::Point &startPt);

	// FUNCTION NewMenu(menuID: INTEGER; menuTitle: Str255): MenuHandle;
	// NewMenu allocates space for a new menu with the given menu ID and tide, and returns a handle
	// to it. It sets up the menu to use the standard menu definition procedure. (The menu definition
	// procedure is read into memory if it isn't already in memory.) The new menu (which is created
	// empty) is not installed in the menu list. To use this menu, you must first call AppendMenu or
	// AddResMenu to fill it with items, InsertMenu to place it in the menu list, and DrawMenuBar to
	// update the menu bar to include the new tide.
	MenuHandle NewMenu(uint16 menuID, const Common::U32String &menuTitle);

	// PROCEDURE SetItem(theMenu: MenuHandle; itemString: Str255);
	// SetItem changes the text of the given menu item to itemString. It doesn't recognize the meta-
	// characters used in AppendMenu; if you include them in itemString, they will appear in the text of
	// the menu item. The attributes already in effect for this item—its character style, icon, and so
	// on—remain in effect. ItemString may be blank but should not be the empty string.
	void SetItem(MenuHandle &theMenu, uint16 item, const Common::U32String &itemString);

	// toolbox_pict.cpp

	// PROCEDURE ClosePicture;
	// The ClosePicture procedure stops collecting drawing commands and picture
	// comments for the currently open picture. You should perform one and only one call to
	// ClosePicture for every call to the OpenCPicture (or OpenPicture) function.
	void ClosePicture();

	// PROCEDURE DrawPicture (myPicture: PicHandle; dstRect: Rect);
	// Within the rectangle that you specify in the dstRect parameter, the DrawPicture
	// procedure draws the picture that you specify in the myPicture parameter.
	void DrawPicture(PicHandle &myPicture, const Common::Rect &dstRect);

	// FUNCTION GetPicture (picID: INTEGER) : PicHandle;
	// GetPicture returns a handle to the picture having the given resource ID, reading it from the
	// resource file if necessary. It calls the Resource Manager function GetResource('PICT',picID). If
	// the resource can't be read, GetPicture returns NIL. The PicHandle data type is defined in
	// QuickDraw.
	PicHandle GetPicture(uint16 picID);

	// PROCEDURE KillPicture(myPicture: PicHandle);
	// The KillPicture procedure releases the memory occupied by the picture whose
	// handle you pass in the myPicture parameter. Use this only when you’re completely
	// finished with a picture.
	void KillPicture(PicHandle &myPicture);

	// FUNCTION OpenPicture (picFrame: Rect): PicHandle;
	// OpenPicture returns a handle to a new picture that has the given rectangle as its picture frame, and
	// tells QuickDraw to start saving as the picture definition all calls to drawing routines and all picture
	// comments (if any).
	PicHandle OpenPicture(const Common::Rect &picFame);

	// toolbox_quickdraw.cpp

	// PROCEDURE BackPat (pat: Pattern);
	// The BackPat procedure sets the bit pattern defined in the Pattern record, which you
	// specify in the pat parameter, to be the background pattern. (The standard bit patterns
	// white, black, gray, ltGray, and dkGray are predefined; the initial background
	// pattern for the graphics port is white.) This pattern is stored in the bkPat field of a
	// GrafPort record.
	void BackPat(const Pattern &pat);

	// PROCEDURE BeginUpdate (theWindow: WindowPtr);
	// Call BeginUpdate when an update event occurs for theWindow. BeginUpdate replaces the
	// visRgn of the window's grafPort with the intersection of the visRgn and the update region and
	// then sets the window's update region to an empty region. You would then usually draw the
	// entire content region, though it suffices to draw only the visRgn; in either case, only the parts of
	// the window that require updating will actually be drawn on the screen. Every call to
	// BeginUpdate must be balanced by a call to EndUpdate.
	void BeginUpdate(WindowRecord &theWindow);

	// PROCEDURE ClipRect (r: Rect);
	// ClipRect changes the clipping region of the current grafPort to a rectangle that's equivalent to the
	// given rectangle. Note that this doesn't change the region handle, but affects the clipping region
	// itself.
	void ClipRect(Common::Rect &r);

	// PROCEDURE ClosePoly;
	// ClosePoly tells QuickDraw to stop saving the definition of the currently open polygon and computes
	// the polyBBox rectangle. You should perform one and only one ClosePoly for every OpenPoly.
	// ClosePoly calls ShowPen, balancing the HidePen call made by OpenPoly.
	void ClosePoly();

	// PROCEDURE CopyBits (srcBits,dstBits: BitMap; srcRect,dstRect: Rect; mode: INTEGER; maskRgn: RgnHandle);
	// CopyBits transfers a bit image between any two bit maps and clips the result to the area specified
	// by the maskRgn parameter. The transfer may be performed in any of the eight source transfer
	// modes. The result is always clipped to the maskRgn and the boundary rectangle of the
	// destination bit map; if the destination bit map is the current grafPort's portBits, it's also clipped to
	// the intersection of the grafPort's clipRgn and visRgn. If you don't want to clip to a maskRgn,
	// just pass NIL for the maskRgn parameter. The dstRect and maskRgn coordinates are in terms of
	// the dstBits.bounds coordinate system, and the srcRect coordinates are in terms of the
	// srcBits.bounds coordinates.
	void CopyBits(const BitMap &srcBits, BitMap &dstBits, const Common::Rect &srcRect, const Common::Rect &dstRect, SourceMode mode, RgnHandle maskRgn);

	// PROCEDURE EraseOval (r: Rect);
	// Using the background pattern for the current graphics port and the patCopy pattern
	// mode, the EraseOval procedure draws the interior of an oval just inside the bounding
	// rectangle that you specify in the r parameter. This effectively erases the oval bounded by
	// the specified rectangle.
	void EraseOval(const Common::Rect &r);

	// PROCEDURE ErasePoly (poly: PolyHandle);
	// Using the patCopy pattern mode, the ErasePoly procedure draws the interior of the
	// polygon whose handle you pass in the poly parameter with the background pattern for
	// the current graphics port.
	void ErasePoly(const PolyHandle &poly);

	// PROCEDURE EraseRect (r: Rect);
	// Using the patCopy pattern mode, the EraseRect procedure draws the interior of the
	// rectangle that you specify in the r parameter with the background pattern for the current
	// graphics port. This effectively erases the rectangle specified in the r parameter. For
	// example, you can use EraseRect to erase the port rectangle for a window before
	// redrawing into the window.
	void EraseRect(const Common::Rect &r);

	// PROCEDURE EraseRoundRect (r: Rect; ovalWidth,ovalHeight: Integer);
	// Using the patCopy pattern mode, the EraseRoundRect procedure draws the interior of
	// the rounded rectangle bounded by the rectangle that you specify in the r parameter with
	// the background pattern of the current graphics port. This effectively erases the rounded
	// rectangle. Use the ovalWidth and ovalHeight parameters to specify the diameters of
	// curvature for the corners of the rounded rectangle.
	void EraseRoundRect(const Common::Rect &r, uint16 ovalWidth, uint16 ovalHeight);

	// PROCEDURE EndUpdate (theWindow: WindowPtr);
	// Call EndUpdate to restore the normal visRgn of theWindow's grafPort, which was changed by
	// BeginUpdate as described above.
	void EndUpdate(WindowRecord &theWindow);

	// PROCEDURE FillOval (r: Rect; pat: Pattern);
	// FillOval fills an oval just inside the specified rectangle with the given pattern (in patCopy mode).
	// The grafPort's pnPat, pnMode, and bkPat are all ignored; the pen location is not changed.
	void FillOval(const Common::Rect &r, const Pattern &pat);

	// PROCEDURE FillPoly (poly: PolyHandle; pat: Pattern);
	// Using the patCopy pattern mode, the FillPoly procedure draws the interior of the
	// polygon whose handle you pass in the poly parameter with the pattern defined in the
	// Pattern record that you specify in the pat parameter.
	void FillPoly(const PolyHandle &poly, const Pattern &pat);

	// PROCEDURE FillRect (r: Rect; pat: Pattern);
	// FillRect fills the specified rectangle with the given pattern (in patCopy mode). The grafPort's
	// pnPat, pnMode, and bkPat are all ignored; the pen location is not changed.
	void FillRect(const Common::Rect &r, const Pattern &pat);

	// PROCEDURE FillRoundRect (r: Rect; ovalWidth, ovalHeight: INTEGER, pat: Pattern);
	// FillRoundRect fills the specified rounded-comer rectangle with the given pattern (in patCopy
	// mode). OvalWidth and ovalHeight specify the diameters of curvature for the corners. The
	// grafPort's pnPat, pnMode, and bkPat are all ignored; the pen location is not changed.
	void FillRoundRect(const Common::Rect &r, uint16 ovalWidth, uint16 ovalHeight, const Pattern &pat);

	// PROCEDURE FrameArc (r: Rect; startAngle,arcAngle: INTEGER);
	// FrameArc draws an arc of the oval that fits inside the specified rectangle, using the current
	// grafPort's pen pattern, mode, and size. StartAngle indicates where the arc begins and is treated
	// MOD 360. ArcAngle defines the extent of the arc. The angles are given in positive or negative
	// degrees; a positive angle goes clockwise, while a negative angle goes counterclockwise.
	void FrameArc(const Common::Rect &r, int16 startAngle, int16 arcAngle);

	// PROCEDURE FrameOval (r: Rect);
	// FrameOval draws an outline just inside the oval that fits inside the specified rectangle, using the
	// current grafPort's pen pattern, mode, and size. The outline is as wide as the pen width and as tall
	// as the pen height. It's drawn with the pnPat, according to the pattern transfer mode specified by
	// pnMode. The pen location is not changed by this procedure.
	void FrameOval(const Common::Rect &r);

	// PROCEDURE FramePoly (poly: PolyHandle);
	// Using the current graphics port’s pen pattern, pattern mode, and size, the FramePoly
	// procedure plays back the line-drawing commands that define the polygon whose handle
	// you pass in the poly parameter.
	void FramePoly(const PolyHandle &poly);

	// PROCEDURE FrameRect (r: Rect);
	// FrameRect draws an outline just inside the specified rectangle, using the current grafPort's pen
	// pattern, mode, and size. The outline is as wide as the pen width and as tall as the pen height. It's
	// drawn with the pnPat, according to the pattern transfer mode specified by pnMode. The pen
	// location is not changed by this procedure.
	void FrameRect(const Common::Rect &r);

	// PROCEDURE FrameRoundRect (r: Rect; ovalWidth,ovalHeight: Integer);
	// Using the pattern, pattern mode, and size of the graphics pen for the current graphics
	// port, the FrameRoundRect procedure draws an outline just inside the rounded
	// rectangle bounded by the rectangle that you specify in the r parameter. The outline is as
	// wide as the pen width and as tall as the pen height. The pen location does not change.
	void FrameRoundRect(const Common::Rect &r, uint16 ovalWidth, uint16 ovalHeight);

	// PROCEDURE GetCPixel (h,v: INTEGER; VAR cPix: RGBColor);
	// The GetCPixel function returns the RGB of the pixel at the specified position in the current
	// port.
	void GetCPixel(int16 h, int16 v, RGBColor &cPix);

	// FUNCTION GetIcon(iconID: INTEGER): Handle;
	// GetIcon returns a handle to the icon having the given resource ID, reading it from the resource file
	// if necessary. It calls the Resource Manager function GetResource('ICON ,iconID). If the
	// resource can't be read, GetIcon returns NIL.
	Handle GetIcon(uint16 iconID);

	// PROCEDURE GetPort (VAR port: GrafPtr);
	// The GetPort procedure returns a pointer to the current graphics port in the port
	// parameter. The current graphics port is also available through the global variable
	// thePort, but you may prefer to use GetPort for better readability of your code. For
	// example, your program could include GetPort(savePort) before setting a new
	// graphics port, followed by SetPort(savePort) to restore the previous port.
	void GetPort(GrafPtr &port);

	// PROCEDURE GlobalToLocal (VAR pt: Point);
	// GlobalToLocal takes a point expressed in global coordinates (with the top left corner of the bit
	// image as coordinate (0,0)) and converts it into the local coordinates of the current grafPort. The
	// global point can be obtained with the LocalToGlobal call (see above).
	void GlobalToLocal(Common::Point &pt);

	// PROCEDURE HideCursor;
	// HideCursor removes the cursor from the screen, restoring the bits under it, and decrements the
	// cursor level (which InitCursor initialized to 0). Every call to HideCursor should be balanced by a
	// subsequent call to ShowCursor.
	void HideCursor();

	// PROCEDURE HidePen;
	// HidePen decrements the current grafPort's pnVis field, which is initialized to 0 by OpenPort;
	// whenever pnVis is negative, the pen doesn't draw on the screen. PnVis keeps track of the
	// number of times the pen has been hidden to compensate for nested calls to HidePen and ShowPen
	// (below). Every call to HidePen should be balanced by a subsequent call to ShowPen. HidePen
	// is called by OpenRgn, OpenPicture, and OpenPoly so that you can define regions, pictures, and
	// polygons without drawing on the screen.
	void HidePen();

	// PROCEDURE InitCursor;
	// InitCursor sets the current cursor to the standard arrow and sets the cursor level to 0, making
	// the cursor visible. The cursor level keeps track of the number of times the cursor has been
	// hidden to compensate for nested calls to HideCursor and ShowCursor, explained below.
	void InitCursor();

	// PROCEDURE InsetRect (VAR r: Rect; dh,dv: INTEGER);
	// InsetRect shrinks or expands the given rectangle. The left and right sides are moved in by the
	// amount specified by dh; the top and bottom are moved toward the center by the amount specified
	// by dv. If dh or dv is negative, the appropriate pair of sides is moved outward instead of inward.
	// The effect is to alter the size by 2*dh horizontally and 2*dv vertically, with the rectangle
	// remaining centered in the same place on the coordinate plane.
	void InsetRect(Common::Rect &r, int16 dh, int16 dv);

	// PROCEDURE InvertOval (r: Rect);
	// InvertOval inverts the pixels enclosed by an oval just inside the specified rectangle: Every white
	// pixel becomes black and every black pixel becomes white. The grafPort's pnPat, pnMode, and
	// bkPat are all ignored; the pen location is not changed.
	void InvertOval(const Common::Rect &r);

	// PROCEDURE InvertPoly (poly: PolyHandle);
	// The InvertPoly procedure inverts the pixels enclosed by the polygon whose handle
	// you pass in the poly parameter. Every white pixel becomes black and every black pixel
	// becomes white.
	void InvertPoly(const PolyHandle &poly);

	// PROCEDURE InvertRect (r: Rect);
	// InvertRect inverts the pixels enclosed by the specified rectangle: Every white pixel becomes
	// black and every black pixel becomes white. The grafPort's pnPat, pnMode, and bkPat are all
	// ignored; the pen location is not changed.
	void InvertRect(const Common::Rect &r);

	// PROCEDURE InvertRoundRect (r: Rect; ovalWidth, ovalHeight: Integer);
	// The InvertRoundRect procedure inverts the pixels enclosed by the rounded rectangle
	// bounded by the rectangle that you specify in the r parameter. Every white pixel becomes
	// black and every black pixel becomes white. The ovalWidth and ovalHeight
	// parameters specify the diameters of curvature for the corners. The pen location does not
	// change.
	void InvertRoundRect(const Common::Rect &r, uint16 ovalWidth, uint16 ovalHeight);

	// PROCEDURE KillPoly (poly: PolyHandle);
	// KillPoly releases the memory occupied by the given polygon. Use this only when you're completely
	// through with a polygon.
	void KillPoly(PolyHandle &poly);

	// PROCEDURE Line(dh, dv: INTEGER);
	// This procedure draws a line to the location that's a distance of dh horizontally and dv vertically
	// from the current pen location; it calls LineTo(h+dh,v+dv), where (h,v) is the current location.
	// The positive directions are to the right and down. The pen location becomes the coordinates of
	// the end of the line after the line is drawn. See the "General Discussion of Drawing" section.
	void Line(int16 dh, int16 dv);

	// PROCEDURE LineTo (h,v: INTEGER);
	// LineTo draws a line from the current pen location to the location specified (in local coordinates)
	// by h and v. The new pen location is (h,v) after the line is drawn.
	void LineTo(int16 h, int16 v);

	// PROCEDURE LocalToGlobal (VAR pt: Point);
	// LocalToGlobal converts the given point from the current grafPort's local coordinate system into a
	// global coordinate system with the origin (0,0) at the top left corner of the port's bit image (such
	// as the screen). This global point can then be compared to other global points, or be changed into
	// the local coordinates of another grafPort.
	void LocalToGlobal(Common::Point &pt);

	// PROCEDURE Move (dh,dv: INTEGER);
	// The Move procedure moves the graphics pen from its current location in the current
	// graphics port a horizontal distance that you specify in the dh parameter and a vertical
	// distance that you specify in the dv parameter.
	void Move(int16 dh, int16 dv);

	// PROCEDURE MovePortTo (leftGlobal,topGlobal: INTEGER);
	// MovePortTo changes the position of the current grafPort's portRect. This does not affect the
	// screen; it merely changes the location at which subsequent drawing inside the port will appear.
	void MovePortTo(int16 leftGlobal, int16 topGlobal);

	// PROCEDURE MoveTo (h,v: INTEGER);
	// MoveTo moves the pen to location (h,v) in the local coordinates of the current grafPort. No
	// drawing is performed.
	void MoveTo(int16 h, int16 v);

	// FUNCTION OpenPoly : PolyHandle;
	// OpenPoly returns a handle to a new polygon and tells QuickDraw to start saving the polygon
	// definition as specified by calls to line-drawing routines. While a polygon is open, all calls to Line
	// and LineTo affect the outline of the polygon. Only the line endpoints affect the polygon
	// definition; the pen mode, pattern, and size do not affect it. In fact, OpenPoly calls HidePen, so
	// no drawing occurs on the screen while the polygon is open (unless you call ShowPen just after
	// OpenPoly, or you called ShowPen previously without balancing it by a call to HidePen).
	PolyHandle OpenPoly();

	// PROCEDURE OpenPort (port: GrafPtr);
	// OpenPort allocates space for the given grafPort's visRgn and clipRgn, initializes the fields of the
	// grafPort as indicated below, and makes the grafPort the current port (by calling SetPort).
	// OpenPort is called by the Window Manager when you create a window, and you normally won't
	// call it yourself. If you do call OpenPort, you can create the grafPtr with the Memory Manager
	// procedure NewPtr or reserve the space on the stack (with a variable of type GrafPort).
	void OpenPort(GrafPtr port);

	// PROCEDURE PaintOval (r: Rect);
	// PaintOval paints an oval just inside the specified rectangle with the current grafPort's pen pattern
	// and mode. The oval is filled with the pnPat, according to the pattern transfer mode specified by
	// pnMode. The pen location is not changed by this procedure.
	void PaintOval(const Common::Rect &r);

	// PROCEDURE PaintPoly (poly: PolyHandle);
	// PaintPoly paints the specified polygon with the current grafPort's pen pattern and pen mode. The
	// polygon is filled with the pnPat, according to the pattern transfer mode specified by pnMode.
	// The pen location is not changed by this procedure.
	void PaintPoly(const PolyHandle &poly);

	// PROCEDURE PaintRect (r: Rect);
	// PaintRect paints the specified rectangle with the current grafPoit's pen pattern and mode. The
	// rectangle is filled with the pnPat, according to the pattern transfer mode specified by pnMode.
	// The pen location is not changed by this procedure.
	void PaintRect(const Common::Rect &r);

	// PROCEDURE PaintRoundRect (r: Rect; ovalWidth,ovalHeight: Integer);
	// Using the pattern and pattern mode of the graphics pen for the current graphics port, the
	// PaintRoundRect procedure draws the interior of the rounded rectangle bounded by
	// the rectangle that you specify in the r parameter. Use the ovalWidth and ovalHeight
	// parameters to specify the diameters of curvature for the corners of the rounded rectangle.
	void PaintRoundRect(const Common::Rect &r, uint16 ovalWidth, uint16 ovalHeight);

	// PROCEDURE PenMode (mode: INTEGER);
	// PenMode sets the transfer mode through which the pen pattern is transferred onto the bit map
	// when lines or shapes are drawn in the current grafPort.
	void PenMode(PatternMode mode);

	// PROCEDURE PenNormal;
	// PenNormal resets the initial state of the pen in the current grafPort.
	void PenNormal();

	// PROCEDURE PenPat (pat: Pattern);
	// PenPat sets the pattern that's used by the pen in the current grafPort. The standard patterns
	// white, black, gray, ltGray, and dkGray are predefined; the initial pen pattern is black. The
	// current pen pattern can be accessed in the variable thePort .pnPat, and this value can be assigned
	// to any other variable of type Pattern.
	void PenPat(const Pattern &pat);

	// PROCEDURE PenSize (width,height: INTEGER);
	// PenSize sets the dimensions of the graphics pen in the current grafPort. All subsequent calls to
	// Line, LineTo, and the procedures that draw framed shapes in the current grafPort will use the
	// new pen dimensions.
	void PenSize(uint16 width, uint16 height);

	// PROCEDURE PlotIcon (theRect: Rect; theIcon: Handle);
	// Plotlcon draws the icon whose handle is thelcon in the rectangle theRect, which is in the local
	// coordinates of the current grafPort. It calls the QuickDraw procedure CopyBits and uses the
	// srcCopy transfer mode.
	void PlotIcon(const Common::Rect &theRect, const Handle &theIcon);

	// PROCEDURE PortSize (width,height: INTEGER);
	// PortSize changes the size of the current grafPort's portRect. This does not affect the screen; it
	// merely changes the size of the "active area" of the grafPort.
	void PortSize(uint16 width, uint16 height);

	// FUNCTION PtInRect (pt: Point; r: Rect): Boolean;
	// The PtInRect function determines whether the pixel below and to the right of the point
	// you specify in the pt parameter is enclosed in the rectangle that you specify in the Rect
	// parameter. The PtInRect function returns TRUE if it is, FALSE if it is not.
	bool PtInRect(const Common::Point &pt, const Common::Rect &r);

	// PROCEDURE SetCPixel (h,v: INTEGER; cPix: RGBColor);
	// The SetCPixel function sets the pixel at the specified position to the pixel value that most
	// closely matches the specified RGB.
	void SetCPixel(int16 h, int16 v, const RGBColor &cPix);

	void SetCursor(const Common::SharedPtr<Cursor> &crsr);

	// PROCEDURE SetOrigin(h, v: Integer);
	// The SetOrigin procedure changes the coordinates of the upper-left corner of the
	// current graphics port’s port rectangle to the values supplied by the h and v parameters.
	// All other points in the current graphics port’s local coordinate system are calculated from
	// this point. All subsequent drawing and calculation routines use the new coordinate
	// system.
	void SetOrigin(int16 h, int16 v);

	// PROCEDURE SetPort (port: GrafPtr);
	// SetPort makes the specified grafPort the current port.
	void SetPort(GrafPtr port);

	// PROCEDURE SetPortBits (bm: BitMap);
	// SetPortBits sets the portBits field of the current grafPort to any previously defined bit map. This
	// allows you to perform all normal drawing and calculations on a buffer other than the screen—for
	// example, a small off-screen image for later "stamping" onto the screen (with the CopyBits
	// procedure, described under "Bit Transfer Operations" below).
	void SetPortBits(BitMap &bm);

	// PROCEDURE SetRect (VAR r: Rect; left,top,right,bottom: INTEGER);
	// SetRect assigns the four boundary coordinates to the given rectangle. The result is a rectangle
	// with coordinates (left,top) (right,bottom).
	void SetRect(Common::Rect &r, int16 left, int16 top, int16 right, int16 bottom);

	// PROCEDURE ShowCursor;
	// ShowCursor increments the cursor level, which may have been decremented by HideCursor, and
	// displays the cursor on the screen if the level becomes 0. A call to ShowCursor should balance
	// each previous call to HideCursor. The level isn't incremented beyond 0, so extra calls to
	// ShowCursor have no effect.
	void ShowCursor();

	// PROCEDURE ShowPen;
	// ShowPen increments the current grafPort's pnVis field, which may have been decremented by
	// HidePen; if pnVis becomes 0, QuickDraw resumes drawing on the screen. Extra calls to
	// ShowPen will increment pnVis beyond 0, so every call to ShowPen should be balanced by a call
	// to HidePen. ShowPen is called by CloseRgn, ClosePicture, and ClosePoly.
	void ShowPen();

	// toolbox_text.cpp

	// PROCEDURE DrawChar (ch: CHAR);
	// DrawChar places the given character to the right of the pen location, with the left end of its base
	// line at the pen's location, and advances the pen accordingly. If the character isn't in the font, the
	// font's missing symbol is drawn.
	void DrawChar(Common::u32char_type_t ch);

	// PROCEDURE DrawString (s: Str255);
	// DrawString calls DrawChar for each character in the given string. The string is placed beginning
	// at the current pen location and extending right. No formatting (such as carriage returns and line
	// feeds) is performed by QuickDraw. The pen location ends up to the right of the last character in
	// the string.
	void DrawString(const Common::U32String &s);

	// PROCEDURE StringWidth (s: Str255) : INTEGER;
	// StringWidth returns the width of the given text string, which it calculates by adding the
	// CharWidths of all the characters in the string (see above).
	uint16 StringWidth(const Common::U32String &s);

	// PROCEDURE TextFace (face: Style);
	// The TextFace procedure sets the style of the font in which the text is to be drawn in the
	// current graphics port.
	void TextFace(uint16 style);

	// PROCEDURE TextFont (font: Integer);
	// The TextFont procedure sets the font of the current graphics port in which the text is
	// to be rendered.
	void TextFont(uint16 font);

	// PROCEDURE TextMode (mode: Integer);
	// The TextMode procedure sets the transfer mode for drawing text in the current
	// graphics port.
	void TextMode(SourceMode mode);

	// PROCEDURE TextSize (size: Integer);
	// The TextSize procedure sets the font size for text drawn in the current graphics port to
	// the speciﬁed number of points.
	void TextSize(uint16 size);

private:
	Common::HashMap<int16, Common::SharedPtr<Common::MacResManager>> _resMap;
	Common::Array<int16> _resOrder;
	int16 _nextResId = 1;
	size_t _resIndexStart = 0;
	Common::HashMap<Handle, ToolboxResInfo> _resInfo;
	Common::HashMap<PicHandle, Handle> _resPicts;
	Graphics::FrameLimiter *_frameLimiter = nullptr;
	Common::SharedPtr<Cursor> _cursor;
	int _cursorLevel = 0;
	Common::Point _mouse;
	uint16 _modifiers = 0x80;
	Common::HashMap<uint16, MenuHandle> _menu;

	Common::List<EventRecord> _events;

	GrafPtr _port = nullptr;

	void _pumpEvents();
	void _updateScreen();
	void _drawOval(const Common::Rect &r, const Pattern &pat, PatternMode mode, bool frame, uint32 fgColor, uint32 bkColor);
	void _drawPoly(const PolyHandle &p, const Pattern &pat, PatternMode mode, bool frame, uint32 fgColor, uint32 bkColor);
	void _drawRect(const Common::Rect &r, const Pattern &pat, PatternMode mode, bool frame, uint32 fgColor, uint32 bkColor);
	void _drawRoundRect(const Common::Rect &r, const Pattern &pat, PatternMode mode, bool frame, uint32 fgColor, uint32 bkColor, uint16 ovalWidth, uint16 ovalHeight);
	void _drawBitsRect(Common::SeekableReadStream &stream, const Common::Rect &picFrame, bool compressed);
	void _copyBits(const BitMap &srcBits, const BitMap &mask, BitMap &dstBits, const Common::Rect &srcRect, const Common::Rect &dstRect, SourceMode mode, RgnHandle maskRgn);
};

} // namespace Fool

#endif
