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

#include "common/hashmap.h"
#include "common/system.h"
#include "common/macresman.h"
#include "common/str.h"
#include "common/ustr.h"

#include "graphics/framelimiter.h"
#include "graphics/macgui/macwindow.h"
#include "graphics/macgui/macwindowmanager.h"

#include "fool/siphash/halfsip.h"

namespace Fool {

struct Region {
	uint16 rgnSize;
	Common::Rect rgnBBox;
};

typedef Common::SharedPtr<Common::Array<byte>> Handle;
typedef Common::SharedPtr<Region> RgnHandle;
typedef Common::SharedPtr<Graphics::ManagedSurface> PicHandle;
// BitMap is the monochrome surface format.
typedef Common::SharedPtr<Graphics::ManagedSurface> BitMap;
typedef Handle PolyHandle;
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

struct Pattern {
	uint8 data[8];
};

struct RGBColor {
	uint16 red;
	uint16 green;
	uint16 blue;
};

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
	kApp4Evt = 15
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

struct EventRecord {
	EventCode what;
	uint32 message;
	uint32 when;
	Common::Point where;
	uint16 modifiers;
};

enum WindowDefinition {
	kDocumentProc = 0,
	kDBoxProc = 1,
	kPlainDBox = 2,
	kAltDBoxProc = 3,
	kNoGrowDocProc = 4,
	kRDocProc = 16,
};

struct GrafPort {
	uint16 device = 0;
	BitMap portBits;
	Common::Rect portRect;
	RgnHandle visRgn;
	RgnHandle clipRgn;
	Pattern bkPat = { { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf } };
	Pattern fillPat = { { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
	Common::Point pnLoc = {0, 0};
	Common::Point pnSize = {1, 1};
	PatternMode pnMode = kPatCopy;
	Pattern pnPat = { { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
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
	Handle polySave;

};
typedef GrafPort * GrafPtr;

struct WindowRecord {
	GrafPort port;
	uint16 windowKind;
};

// TYPE Rect: top: INTEGER; left: INTEGER; bottom: INTEGER; right: INTEGER;

struct ToolboxResInfo {
	int16 fileID;
	ResType type;
	uint16 resID;
	Handle handle;
	Common::String name;
};

class Toolbox {

public:
	// Compatibility shim for the Macintosh Toolbox/QuickDraw API.
	Toolbox();
	~Toolbox();

	// toolbox.cpp

	// PROCEDURE Delay (numTicks: LONGINT; VAR finalTicks: LONGINT);
	// Delay causes the system to wait for the number of ticks (sixtieths of a second) specified by numTicks, and returns in finalTicks the total number of ticks from system startup to the end of the delay.
	uint32 Delay(uint32 numTicks);

	// FUNCTION GetNextEvent (eventMask: INTEGER; VAR theEvent: EventRecord) : BOOLEAN;
	// GetNextEvent returns the next available event of a specified type or types and, if the event is in
	// the event queue, removes it from the queue. The event is returned in the parameter theEvent. The
	// eventMask parameter specifies which event types are of interest. GetNextEvent returns the next
	// available event of any type designated by the mask, subject to the priority rules discussed above
	// under "Priority of Events". If no event of any of the designated types is available, GetNextEvent
	// returns a null event.
	bool GetNextEvent(uint16 eventMask, EventRecord &theEvent);

	// FUNCTION TickCount : LONGINT;
	// TickCount returns the current number of ticks (sixtieths of a second) since the system last started
	// up.
	uint32 TickCount();


	// toolbox_resman.cpp

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

	// PROCEDURE DisposHandle (h: Handle);
	// DisposHandle releases the memory occupied by the relocatable block whose handle is h.
	void DisposHandle(Handle &h);

	// FUNCTION GetHandleSize (h: Handle) : Size;
	// GetHandleSize returns the logical size, in bytes, of the relocatable block whose handle is h. In
	// case of an error, GetHandleSize returns 0.
	Size GetHandleSize(Handle &h);

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


	// toolbox_quickdraw.cpp

	// PROCEDURE BeginUpdate (theWindow: WindowPtr);
	// Call BeginUpdate when an update event occurs for theWindow. BeginUpdate replaces the
	// visRgn of the window's grafPort with the intersection of the visRgn and the update region and
	// then sets the window's update region to an empty region. You would then usually draw the
	// entire content region, though it suffices to draw only the visRgn; in either case, only the parts of
	// the window that require updating will actually be drawn on the screen. Every call to
	// BeginUpdate must be balanced by a call to EndUpdate.
	void BeginUpdate(WindowRecord &theWindow);

	// PROCEDURE ClearMenuBar;
	// Call ClearMenuBar to remove all menus from the menu list when you want to start afresh with all
	// new menus. Be sure to call DrawMenuBar to update the menu bar.
	void ClearMenuBar();

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
	void DrawString(const Common::String &s);

	// PROCEDURE EndUpdate (theWindow: WindowPtr);
	// Call EndUpdate to restore the normal visRgn of theWindow's grafPort, which was changed by
	// BeginUpdate as described above.
	void EndUpdate(WindowRecord &theWindow);

	// PROCEDURE FillOval (r: Rect; pat: Pattern);
	// FillOval fills an oval just inside the specified rectangle with the given pattern (in patCopy mode).
	// The grafPort's pnPat, pnMode, and bkPat are all ignored; the pen location is not changed.
	void FillOval(const Common::Rect &r, const Pattern &pat);

	// PROCEDURE FillRect (r: Rect; pat: Pattern);
	// FillRect fills the specified rectangle with the given pattern (in patCopy mode). The grafPort's
	// pnPat, pnMode, and bkPat are all ignored; the pen location is not changed.
	void FillRect(const Common::Rect &r, const Pattern &pat);

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

	// PROCEDURE FrameRect (r: Rect);
	// FrameRect draws an outline just inside the specified rectangle, using the current grafPort's pen
	// pattern, mode, and size. The outline is as wide as the pen width and as tall as the pen height. It's
	// drawn with the pnPat, according to the pattern transfer mode specified by pnMode. The pen
	// location is not changed by this procedure.
	void FrameRect(const Common::Rect &r);

	// PROCEDURE GetCPixel (h,v: INTEGER; VAR cPix: RGBColor);
	// The GetCPixel function returns the RGB of the pixel at the specified position in the current
	// port.
	void GetCPixel(int16 h, int16 v, RGBColor &cPix);

	// FUNCTION GetPicture (picID: INTEGER) : PicHandle;
	// GetPicture returns a handle to the picture having the given resource ID, reading it from the
	// resource file if necessary. It calls the Resource Manager function GetResource('PICT',picID). If
	// the resource can't be read, GetPicture returns NIL. The PicHandle data type is defined in
	// QuickDraw.
	PicHandle GetPicture(uint16 picID);

	// PROCEDURE HideCursor;
	// HideCursor removes the cursor from the screen, restoring the bits under it, and decrements the
	// cursor level (which InitCursor initialized to 0). Every call to HideCursor should be balanced by a
	// subsequent call to ShowCursor.
	void HideCursor();

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

	// PROCEDURE InvertRect (r: Rect);
	// InvertRect inverts the pixels enclosed by the specified rectangle: Every white pixel becomes
	// black and every black pixel becomes white. The grafPort's pnPat, pnMode, and bkPat are all
	// ignored; the pen location is not changed.
	void InvertRect(const Common::Rect &r);

	// PROCEDURE KillPoly (poly: PolyHandle);
	// KillPoly releases the memory occupied by the given polygon. Use this only when you're completely
	// through with a polygon.
	void KillPoly(PolyHandle poly);

	// PROCEDURE LineTo (h,v: INTEGER);
	// LineTo draws a line from the current pen location to the location specified (in local coordinates)
	// by h and v. The new pen location is (h,v) after the line is drawn.
	void LineTo(int16 h, int16 v);

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
	void PaintPoly(PolyHandle poly);

	// PROCEDURE PaintRect (r: Rect);
	// PaintRect paints the specified rectangle with the current grafPoit's pen pattern and mode. The
	// rectangle is filled with the pnPat, according to the pattern transfer mode specified by pnMode.
	// The pen location is not changed by this procedure.
	void PaintRect(const Common::Rect &r);

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

	// PROCEDURE PortSize (width,height: INTEGER);
	// PortSize changes the size of the current grafPort's portRect. This does not affect the screen; it
	// merely changes the size of the "active area" of the grafPort.
	void PortSize(uint16 width, uint16 height);

	// PROCEDURE SetCPixel (h,v: INTEGER; cPix: RGBColor);
	// The SetCPixel function sets the pixel at the specified position to the pixel value that most
	// closely matches the specified RGB.
	void SetCPixel(int16 h, int16 v, const RGBColor &cPix);

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

	// PROCEDURE StringWidth (s: Str255) : INTEGER;
	// StringWidth returns the width of the given text string, which it calculates by adding the
	// CharWidths of all the characters in the string (see above).
	uint16 StringWidth(const Common::String &s);

private:
	Common::HashMap<int16, Common::SharedPtr<Common::MacResManager>> _resMap;
	Common::Array<int16> _resOrder;
	int16 _nextResId = 1;
	size_t _resIndexStart = 0;
	Common::HashMap<Handle, ToolboxResInfo> _resInfo;
	Common::HashMap<PicHandle, Handle> _resPicts;
	Graphics::FrameLimiter *_frameLimiter = nullptr;
	int _cursorLevel = 0;

	Common::Queue<EventRecord> _events;

	GrafPtr _port = nullptr;

	Graphics::MacPlotData _pd;

	void _pumpEvents();
	void _updateScreen();
};

} // namespace Fool

#endif
