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

#include "common/array.h"
#include "common/events.h"
#include "common/str.h"

namespace Fool {

class Toolbox {

public:
	// PROCEDURE BeginUpdate (theWindow: WindowPtr);
	// Call BeginUpdate when an update event occurs for theWindow. BeginUpdate replaces the
	// visRgn of the window's grafPort with the intersection of the visRgn and the update region and
	// then sets the window's update region to an empty region. You would then usually draw the
	// entire content region, though it suffices to draw only the visRgn; in either case, only the parts of
	// the window that require updating will actually be drawn on the screen. Every call to
	// BeginUpdate must be balanced by a call to EndUpdate. (See "How a Window is Drawn".)

	// PROCEDURE ClearMenuBar;
	// Call ClearMenuBar to remove all menus from the menu list when you want to start afresh with all
	// new menus. Be sure to call DrawMenuBar to update the menu bar.

	// PROCEDURE ClipRect (r: Rect);
	// ClipRect changes the clipping region of the current grafPort to a rectangle that's equivalent to the
	// given rectangle. Note that this doesn't change the region handle, but affects the clipping region
	// itself.

	// PROCEDURE ClosePoly;
	// ClosePoly tells QuickDraw to stop saving the definition of the currently open polygon and computes
	// the polyBBox rectangle. You should perform one and only one ClosePoly for every OpenPoly.
	// ClosePoly calls ShowPen, balancing the HidePen call made by OpenPoly.

	// PROCEDURE CopyBits (srcBits,dstBits: BitMap; srcRect,dstRect: Rect; mode: INTEGER; maskRgn: RgnHandle);
	// CopyBits transfers a bit image between any two bit maps and clips the result to the area specified
	// by the maskRgn parameter. The transfer may be performed in any of the eight source transfer
	// modes. The result is always clipped to the maskRgn and the boundary rectangle of the
	// destination bit map; if the destination bit map is the current grafPort's portBits, it's also clipped to
	// the intersection of the grafPort's clipRgn and visRgn. If you don't want to clip to a maskRgn,
	// just pass NIL for the maskRgn parameter. The dstRect and maskRgn coordinates are in terms of
	// the dstBits.bounds coordinate system, and the srcRect coordinates are in terms of the
	// srcBits.bounds coordinates.

	// FUNCTION CurResFile: INTEGER;
	// CurResFile returns the reference number of the current resource file. You can call it when the
	// application starts up to get the reference number of its resource file.

	// PROCEDURE DrawChar (ch: CHAR);
	// DrawChar places the given character to the right of the pen location, with the left end of its base
	// line at the pen's location, and advances the pen accordingly. If the character isn't in the font, the
	// font's missing symbol is drawn.

	// PROCEDURE DrawString (s: Str255);
	// DrawString calls DrawChar for each character in the given string. The string is placed beginning
	// at the current pen location and extending right. No formatting (such as carriage returns and line
	// feeds) is performed by QuickDraw. The pen location ends up to the right of the last character in
	// the string.

	// PROCEDURE EndUpdate (theWindow: WindowPtr);
	// Call EndUpdate to restore the normal visRgn of theWindow's grafPort, which was changed by
	// BeginUpdate as described above.

	// PROCEDURE FillOval (r: Rect; pat: Pattern);
	// FillOval fills an oval just inside the specified rectangle with the given pattern (in patCopy mode).
	// The grafPort's pnPat, pnMode, and bkPat are all ignored; the pen location is not changed.

	// PROCEDURE FillRect (r: Rect; pat: Pattern);
	// FillRect fills the specified rectangle with the given pattern (in patCopy mode). The grafPort's
	// pnPat, pnMode, and bkPat are all ignored; the pen location is not changed.

	// PROCEDURE FrameArc (r: Rect; startAngle,arcAngle: INTEGER);
	// FrameArc draws an arc of the oval that fits inside the specified rectangle, using the current
	// grafPort's pen pattern, mode, and size. StartAngle indicates where the arc begins and is treated
	// MOD 360. ArcAngle defines the extent of the arc. The angles are given in positive or negative
	// degrees; a positive angle goes clockwise, while a negative angle goes counterclockwise.

	// PROCEDURE FrameOval (r: Rect);
	// FrameOval draws an outline just inside the oval that fits inside the specified rectangle, using the
	// current grafPort's pen pattern, mode, and size. The outline is as wide as the pen width and as tall
	// as the pen height. It's drawn with the pnPat, according to the pattern transfer mode specified by
	// pnMode. The pen location is not changed by this procedure.

	// PROCEDURE FrameRect (r: Rect);
	// FrameRect draws an outline just inside the specified rectangle, using the current grafPort's pen
	// pattern, mode, and size. The outline is as wide as the pen width and as tall as the pen height. It's
	// drawn with the pnPat, according to the pattern transfer mode specified by pnMode. The pen
	// location is not changed by this procedure.

	// PROCEDURE GetCPixel (h,v: INTEGER; VAR cPix: RGBColor);
	// The GetCPixel function returns the RGB of the pixel at the specified position in the current
	// port.

	// FUNCTION GetNextEvent (eventMask: INTEGER; VAR theEvent: EventRecord) : BOOLEAN;
	// GetNextEvent returns the next available event of a specified type or types and, if the event is in
	// the event queue, removes it from the queue. The event is returned in the parameter theEvent. The
	// eventMask parameter specifies which event types are of interest. GetNextEvent returns the next
	// available event of any type designated by the mask, subject to the priority rules discussed above
	// under "Priority of Events". If no event of any of the designated types is available, GetNextEvent
	// returns a null event.

	// FUNCTION GetPicture (picID: INTEGER) : PicHandle;
	// GetPicture returns a handle to the picture having the given resource ID, reading it from the
	// resource file if necessary. It calls the Resource Manager function GetResource('PICT',picID). If
	// the resource can't be read, GetPicture returns NIL. The PicHandle data type is defined in
	// QuickDraw.

	// PROCEDURE HideCursor;
	// HideCursor removes the cursor from the screen, restoring the bits under it, and decrements the
	// cursor level (which InitCursor initialized to 0). Every call to HideCursor should be balanced by a
	// subsequent call to ShowCursor.

	// PROCEDURE InitCursor;
	// InitCursor sets the current cursor to the standard arrow and sets the cursor level to 0, making
	// the cursor visible. The cursor level keeps track of the number of times the cursor has been
	// hidden to compensate for nested calls to HideCursor and ShowCursor, explained below.

	// PROCEDURE InsetRect (VAR r: Rect; dh,dv: INTEGER);
	// InsetRect shrinks or expands the given rectangle. The left and right sides are moved in by the
	// amount specified by dh; the top and bottom are moved toward the center by the amount specified
	// by dv. If dh or dv is negative, the appropriate pair of sides is moved outward instead of inward.
	// The effect is to alter the size by 2*dh horizontally and 2*dv vertically, with the rectangle
	// remaining centered in the same place on the coordinate plane.

	// PROCEDURE InvertOval (r: Rect);
	// InvertOval inverts the pixels enclosed by an oval just inside the specified rectangle: Every white
	// pixel becomes black and every black pixel becomes white. The grafPort's pnPat, pnMode, and
	// bkPat are all ignored; the pen location is not changed.

	// PROCEDURE InvertRect (r: Rect);
	// InvertRect inverts the pixels enclosed by the specified rectangle: Every white pixel becomes
	// black and every black pixel becomes white. The grafPort's pnPat, pnMode, and bkPat are all
	// ignored; the pen location is not changed.

	// PROCEDURE KillPoly (poly: PolyHandle);
	// KillPoly releases the memory occupied by the given polygon. Use this only when you're completely
	// through with a polygon.

	// PROCEDURE LineTo (h,v: INTEGER);
	// LineTo draws a line from the current pen location to the location specified (in local coordinates)
	// by h and v. The new pen location is (h,v) after the line is drawn.

	// PROCEDURE MovePortTo (leftGlobal,topGlobal: INTEGER);
	// MovePortTo changes the position of the current grafPort's portRect. This does not affect the
	// screen; it merely changes the location at which subsequent drawing inside the port will appear.

	// PROCEDURE MoveTo (h,v: INTEGER);
	// MoveTo moves the pen to location (h,v) in the local coordinates of the current grafPort. No
	// drawing is performed.

	// FUNCTION OpenPoly : PolyHandle;
	// OpenPoly returns a handle to a new polygon and tells QuickDraw to start saving the polygon
	// definition as specified by calls to line-drawing routines. While a polygon is open, all calls to Line
	// and LineTo affect the outline of the polygon. Only the line endpoints affect the polygon
	// definition; the pen mode, pattern, and size do not affect it. In fact, OpenPoly calls HidePen, so
	// no drawing occurs on the screen while the polygon is open (unless you call ShowPen just after
	// OpenPoly, or you called ShowPen previously without balancing it by a call to HidePen).

	// PROCEDURE OpenPort (port: GrafPtr);
	// OpenPort allocates space for the given grafPort's visRgn and clipRgn, initializes the fields of the
	// grafPort as indicated below, and makes the grafPort the current port (by calling SetPort).
	// OpenPort is called by the Window Manager when you create a window, and you normally won't
	// call it yourself. If you do call OpenPort, you can create the grafPtr with the Memory Manager
	// procedure NewPtr or reserve the space on the stack (with a variable of type GrafPort).

	// PROCEDURE PaintOval (r: Rect);
	// PaintOval paints an oval just inside the specified rectangle with the current grafPort's pen pattern
	// and mode. The oval is filled with the pnPat, according to the pattern transfer mode specified by
	// pnMode. The pen location is not changed by this procedure.

	// PROCEDURE PaintPoly (poly: PolyHandle);
	// PaintPoly paints the specified polygon with the current grafPort's pen pattern and pen mode. The
	// polygon is filled with the pnPat, according to the pattern transfer mode specified by pnMode.
	// The pen location is not changed by this procedure.

	// PROCEDURE PaintRect (r: Rect);
	// PaintRect paints the specified rectangle with the current grafPoit's pen pattern and mode. The
	// rectangle is filled with the pnPat, according to the pattern transfer mode specified by pnMode.
	// The pen location is not changed by this procedure.

	// PROCEDURE PenMode (mode: INTEGER);
	// PenMode sets the transfer mode through which the pen pattern is transferred onto the bit map
	// when lines or shapes are drawn in the current grafPort.

	// PROCEDURE PenNormal;
	// PenNormal resets the initial state of the pen in the current grafPort.

	// PROCEDURE PenPat (pat: Pattern);
	// PenPat sets the pattern that's used by the pen in the current grafPort. The standard patterns
	// white, black, gray, ltGray, and dkGray are predefined; the initial pen pattern is black. The
	// current pen pattern can be accessed in the variable thePort .pnPat, and this value can be assigned
	// to any other variable of type Pattern.

	// PROCEDURE PenSize (width,height: INTEGER);
	// PenSize sets the dimensions of the graphics pen in the current grafPort. All subsequent calls to
	// Line, LineTo, and the procedures that draw framed shapes in the current grafPort will use the
	// new pen dimensions.

	// PROCEDURE PortSize (width,height: INTEGER);
	// PortSize changes the size of the current grafPort's portRect. This does not affect the screen; it
	// merely changes the size of the "active area" of the grafPort.

	// PROCEDURE ReleaseResource (theResource: Handle);
	// Given a handle to a resource, ReleaseResource releases the memory occupied by the resource
	// data, if any, and replaces the handle to that resource in the resource map with NIL (see Figure 7).
	// The given handle will no longer be recognized as a handle to a resource; if the Resource Manager
	// is subsequendy called to get the released resource, a new handle will be allocated. Use this
	// procedure only after you're completely through with a resource.

	// PROCEDURE SetCPixel (h,v: INTEGER; cPix: RGBColor);
	// The SetCPixel function sets the pixel at the specified position to the pixel value that most
	// closely matches the specified RGB.

	// PROCEDURE SetPort (port: GrafPtr);
	// SetPort makes the specified grafPort the current port.

	// PROCEDURE SetPortBits (bm: BitMap);
	// SetPortBits sets the portBits field of the current grafPort to any previously defined bit map. This
	// allows you to perform all normal drawing and calculations on a buffer other than the screen—for
	// example, a small off-screen image for later "stamping" onto the screen (with the CopyBits
	// procedure, described under "Bit Transfer Operations" below).

	// PROCEDURE SetRect (VAR r: Rect; left,top,right,bottom: INTEGER);
	// SetRect assigns the four boundary coordinates to the given rectangle. The result is a rectangle
	// with coordinates (left,top) (right,bottom).

	// PROCEDURE StringWidth (s: Str255) : INTEGER;
	// StringWidth returns the width of the given text string, which it calculates by adding the
	// CharWidths of all the characters in the string (see above).

	// FUNCTION TickCount : LONGINT;
	// TickCount returns the current number of ticks (sixtieths of a second) since the system last started
	// up.

	// PROCEDURE UseResFile (refNum: INTEGER);
	// Given the reference number of a resource file, UseResFile sets the current resource file to that
	// file. If there's no resource file open with the given reference number, UseResFile will do nothing
	// and the ResError function will return the result code resFNotFound. A refNum of 0 represents
	// the system resource file.
};

} // namespace Fool

#endif
