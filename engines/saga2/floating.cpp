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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL // FIXME: Remove

#include "saga2/std.h"
#include "saga2/rmemfta.h"
#include "saga2/fta.h"
#include "saga2/panel.h"
#include "saga2/images.h"
#include "saga2/floating.h"
#include "saga2/objects.h"
#include "saga2/contain.h"
#include "saga2/imagcach.h"
#include "saga2/grabinfo.h"
#include "saga2/display.h"

namespace Saga2 {

/* classes:
    containerWindow
    mindWindow
    smallContainerWindow
    enchanetmentWindow

    floatingWindow
*/

/* ===================================================================== *
   Prototypes
 * ===================================================================== */

void updateWindowSection(const Rect16 &r);

/* ===================================================================== *
   Imports
 * ===================================================================== */

extern gToolBase    G_BASE;

//  These externs are imported because the window update routines
//  need to know about the scrolling tile area.

extern Rect16       tileRect;
extern gPixelMap    tileDrawMap;
extern Point16      fineScroll;
extern gFont        *mainFont;
extern bool         allPlayerActorsDead;


/* ===================================================================== *
   DecoratedWindow member functions
 * ===================================================================== */

//  Constructor for a background window
DecoratedWindow::DecoratedWindow(const Rect16 &r, uint16 ident, const char saveas[], AppFunc *cmd)
	: gWindow(r, ident, saveas, cmd) {
	decorations = NULL;
	numDecorations = 0;
}

// destructor for decorated windows
DecoratedWindow::~DecoratedWindow(void) {
	removeDecorations();
}

//  Redraw the window, but only a small clipped section,
//  and perhaps drawn onto an off-screen map.

void DecoratedWindow::drawClipped(
    gPort         &port,
    const Point16 &offset,
    const Rect16  &clipRect) {
	WindowDecoration *dec;
	int16           i;

	if (displayEnabled())
		if (extent.overlap(clipRect)) {
			//  For each "decorative panel" within the frame of the window

			for (dec = decorations, i = 0; i < numDecorations; i++, dec++) {
				//  If the decorative panel overlaps the area we are
				//  rendering

				if (dec->extent.overlap(clipRect)) {
					Point16 pos(dec->extent.x - offset.x,
					            dec->extent.y - offset.y);

					drawCompressedImage(port, pos, dec->image);
				}
			}

			gWindow::drawClipped(port, offset, clipRect);
		}

	// REM: We should also draw the scrolling tiles at this time???
}

//  Set up the decorations for the window, and load them into the
//  memory pool

void DecoratedWindow::setDecorations(
    WindowDecoration *dec,
    int16           count,
    hResContext     *con) {
	int16           i;

	decorations = dec;
	numDecorations = count;

	//  For each "decorative panel" within the frame of the window
	for (i = 0; i < numDecorations; i++, dec++) {

		// request an image pointer from the imageCache

		dec->image = ImageCache.requestImage(con,
		                                     MKTAG('B', 'R', 'D', dec->imageNumber));
	}
}

void DecoratedWindow::setDecorations(
    WindowDecoration *dec,
    int16           count,
    hResContext     *con,
    hResID          id) {
	int16           i;

	decorations = dec;
	numDecorations = count;

	//  For each "decorative panel" within the frame of the window

	for (i = 0; i < numDecorations; i++, dec++) {
		// request an image pointer from the image Cache
		dec->image = ImageCache.requestImage(con,
		                                     id | MKTAG(0, 0, 0, dec->imageNumber));
	}
}

void DecoratedWindow::setDecorations(
    WindowDecoration *dec,
    int16           count,
    hResContext     *con,
    char a, char b, char c) {
	setDecorations(dec, count, con, MKTAG(a, b, c, 0));
}

//  Free the decorations from the memory pool

void DecoratedWindow::removeDecorations(void) {
	WindowDecoration *dec;
	int16           i;

	// release requests made to the Image Cache
	for (dec = decorations, i = 0; i < numDecorations; i++, dec++) {
		ImageCache.releaseImage(dec->image);
	}

	decorations = NULL;
	numDecorations = 0;
}

//  Redraw all of the decorations, on the main port only...

void DecoratedWindow::draw(void) {               // redraw the window
	pointer.hide();
	if (displayEnabled())
		drawClipped(mainPort, Point16(0, 0), extent);
	pointer.show();
}

//  Return TRUE if window floats above animated are

bool DecoratedWindow::isBackdrop(void) {
	return FALSE;
}

//  Update a region of a window, and all floaters which
//  might be above that window.
void DecoratedWindow::update(const Rect16 &updateRect) {
	Rect16          r = updateRect;

	r.x += extent.x;
	r.y += extent.y;

	updateWindowSection(r);
}

/* ===================================================================== *
   BackWindow member functions
 * ===================================================================== */

//  Constructor for a background window

BackWindow::BackWindow(const Rect16 &r, uint16 ident, AppFunc *cmd)
	: DecoratedWindow(r, ident, "", cmd) {
}

void BackWindow::invalidate(Rect16 &area) {
	if (displayEnabled())
		window.update(area);
}

//  Return TRUE if window floats above animated are

bool BackWindow::isBackdrop(void) {
	return TRUE;
}

//  A backdrop window can never go to front.

void BackWindow::toFront(void) {}

/* ===================================================================== *
   DragBar class member functions
 * ===================================================================== */

Point16             DragBar::dragOffset,
DragBar::dragPos;
bool                DragBar::update;
FloatingWindow      *DragBar::dragWindow;

DragBar::DragBar(gPanelList &list, const Rect16 &r)
	: gControl(list, r, NULL, 0, NULL) {
}

bool DragBar::activate(gEventType) {
	return TRUE;
}

void DragBar::deactivate(void) {
	gPanel::deactivate();
}

bool DragBar::pointerHit(gPanelMessage &msg) {
	Rect16      wExtent = window.getExtent();

	dragPos.x = wExtent.x;
	dragPos.y = wExtent.y;
	dragOffset = msg.pickAbsPos;

	return TRUE;
}

//  Inform the main drawing loop that a floating window needs to
//  be dragged...

void DragBar::pointerDrag(gPanelMessage &msg) {
	Rect16      ext = window.getExtent();
	Point16     pos;

	//  Calculate new window position

	pos.x = msg.pickAbsPos.x + ext.x - dragOffset.x;
	pos.y = msg.pickAbsPos.y + ext.y - dragOffset.y;

	//  If window position has changed, then signal the drawing loop

	if (pos != dragPos) {
		dragPos = pos;
		update = TRUE;
		dragWindow = (FloatingWindow *)&window;
	}
}

void DragBar::pointerRelease(gPanelMessage &) {
	deactivate();
	update = FALSE;             // just in case
	dragWindow = NULL;
}


/* ===================================================================== *
   gButton class member functions
 * ===================================================================== */

void gButton::deactivate(void) {
	selected = 0;
	draw();
	gPanel::deactivate();
}

bool gButton::activate(gEventType why) {
	selected = 1;
	draw();

	if (why == gEventKeyDown) {             // momentarily depress
#ifndef _WIN32
		g_system->delayMillis(200);
#endif
		deactivate();
		notify(gEventNewValue, 1);       // notify App of successful hit
	}
	return FALSE;
}

bool gButton::pointerHit(gPanelMessage &) {
	activate(gEventMouseDown);
	return TRUE;
}

void gButton::pointerRelease(gPanelMessage &) {
	//  We have to test selected first because deactivate clears it.

	if (selected) {
		deactivate();                       // give back input focus
		notify(gEventNewValue, 1);       // notify App of successful hit
	} else deactivate();
}

void gButton::pointerDrag(gPanelMessage &msg) {
	if (selected != msg.inPanel) {
		selected = msg.inPanel;
		draw();
	}
}

void gButton::draw(void) {
	gPort           &port = window.windowPort;
	Rect16          rect = window.getExtent();

	pointer.hide(port, extent);              // hide mouse pointer
	if (displayEnabled())
		drawClipped(port, Point16(0, 0), Rect16(0, 0, rect.width, rect.height));
	pointer.show(port, extent);              // show mouse pointer
}


/* ===================================================================== *
   gImageButton class member functions
 * ===================================================================== */

void gImageButton::drawClipped(gPort &port, const Point16 &offset, const Rect16 &r) {
	gPixelMap   *currentImage = selected ? selImage : deselImage;

	if (displayEnabled())
		if (extent.overlap(r))
			port.bltPixels(*currentImage,
			               0,
			               0,
			               extent.x - offset.x,
			               extent.y - offset.y,
			               currentImage->size.x,
			               currentImage->size.y);
}

/* ===================================================================== *
   gToggleButton class member functions
 * ===================================================================== */

bool gToggleButton::activate(gEventType why) {
	if (why == gEventKeyDown || why == gEventMouseDown) {
		selected = !selected;
		draw();
		gPanel::deactivate();
		notify(gEventNewValue, selected);    // notify App of successful hit
	}
	return FALSE;
}

bool gToggleButton::pointerHit(gPanelMessage &) {
	return activate(gEventMouseDown);
}

/* ===================================================================== *
   LabeledButton class member functions
 * ===================================================================== */

LabeledButton::LabeledButton(gPanelList &list,
                             const Rect16 &box,
                             gPixelMap &img1,
                             gPixelMap &img2,
                             char *buttonLabel,
                             uint16 ident,
                             AppFunc *cmd) :
	gImageButton(list,
	             box,
	             img1,
	             img2,
	             buttonLabel,
	             ident,
	             cmd) {
	const char *underscore;

	if ((underscore = strchr(title, '_')) != NULL)
		accelKey = toupper(underscore[ 1 ]);
}

void LabeledButton::drawClipped(
    gPort &port,
    const Point16 &offset,
    const Rect16 &r) {
	if (!displayEnabled())  return;
	if (!extent.overlap(r)) return;

	Point16 origin,
	        textOrigin;
	gFont   *textFont = mainFont;

	origin.x = extent.x - offset.x;
	origin.y = extent.y - offset.y;

	SAVE_GPORT_STATE(port);

	port.setColor(14);
	port.fillRect(origin.x, origin.y, extent.width, extent.height);

	gImageButton::drawClipped(port, offset, r);

	textOrigin.x = origin.x + ((extent.width -
	                            TextWidth(textFont, title, -1, textStyleUnderBar)) >> 1);
	textOrigin.y = origin.y + ((extent.height - textFont->height) >> 1);

	port.setColor(2);
	port.moveTo(textOrigin);
	port.setFont(textFont);
	port.setStyle(textStyleUnderBar);
	port.drawText(title, -1);
}


/* ===================================================================== *
   Floating Window class member functions
 * ===================================================================== */

FloatingWindow::FloatingWindow(const Rect16 &r, uint16 ident, const char saveas[], AppFunc *cmd)
	: DecoratedWindow(r, ident, saveas, cmd) {
	VERIFY(db = NEW_UI DragBar(*this, Rect16(0, 0, r.width, r.height)));

	origPos.x = r.x;
	origPos.y = r.y;

#if 0
	decOffset.x = 0;
	decOffset.y = 0;
#endif
}

//  Redraw the window, but only a small clipped section,
//  and perhaps drawn onto an off-screen map.
void FloatingWindow::drawClipped(
    gPort         &port,
    const Point16 &offset,
    const Rect16  &r) {
	Rect16              rect = extent;
	WindowDecoration    *dec;
	int16               i;

	if (displayEnabled())
		if (extent.overlap(r)) {
			// do'nt do the temp stuff if there are decorations present
			if (numDecorations == 0) {
				rect.x -= offset.x;
				rect.y -= offset.y;

				port.setColor(2);
				port.frameRect(rect, 2);
				rect.expand(-2, -2);
				port.setColor(12);
				port.fillRect(rect);
			}

			//  For each "decorative panel" within the frame of the window
			for (dec = decorations, i = 0; i < numDecorations; i++, dec++) {
				Point16 pos(dec->extent.x /* - decOffset.x */ - offset.x + extent.x,
				            dec->extent.y /* - decOffset.y */ - offset.y + extent.y);

				drawCompressedImage(port, pos, dec->image);
			}

			gWindow::drawClipped(port, offset, r);
		}
}

void FloatingWindow::setExtent(const Rect16 &r) {
	// set an offset from the original position of the window
	// for the decorations to use in drawing themselves
#if 0
	decOffset.x = origPos.x - r.x;
	decOffset.y = origPos.y - r.y;
#endif

	// now reset the extent
	extent.height = r.height;
	extent.width  = r.width;
	setPos(Point16(r.x, r.y));
}

bool FloatingWindow::open(void) {
	db->moveToBack(*this);

	mouseInfo.replaceObject();
	mouseInfo.clearGauge();
	mouseInfo.setText(NULL);
	mouseInfo.setIntent(GrabInfo::WalkTo);

	return gWindow::open();
}

//  Close this window and redraw the screen under it.
void FloatingWindow::close(void) {
	gWindow::close();
	updateWindowSection(extent);
}


//  This routine is used by the floating window dragger. It's job
//  is to completely re-render all of the pixels in a rectangular
//  area, includuing any floating windows which might overlap that
//  area, and including the scrolling animated area which might
//  overlap that area.

bool checkTileAreaPort(void);

extern bool userControlsSetup;

void updateWindowSection(const Rect16 &r) {
	DecoratedWindow *dw;
	gPixelMap       tempMap;
	gPort           tempPort;
	Point16         offset(r.x, r.y);
	Rect16          clip,
	                animClip;
	Point16         animOffset(tileRect.x - fineScroll.x, tileRect.y);

	//  Detects that program is shutting down and aborts the blit
	if (tileDrawMap.data == NULL) return;
	if (!checkTileAreaPort()) return;

	//  Since the floating windows can be dragged partly offscreen
	//  we should make sure we're rendering only to on-screen pixels.

	clip = intersect(r, Rect16(0, 0, screenWidth, screenHeight));

	//  Allocate a temporary pixel map and gPort

	tempMap.size.x = clip.width;
	tempMap.size.y = clip.height;
	tempMap.data = (uint8 *)RNewClearPtr(tempMap.bytes(), NULL, "window section");
	if (tempMap.data == NULL) return;
	tempPort.setMap(&tempMap);
	tempPort.setMode(drawModeReplace);

	//  Compute the intersection of the animated area with the clip
	//  rectangle. If they overlap, then copy part of the animated
	//  area into the temporary bitmap.

	animClip = intersect(tileRect, clip);
	if (!animClip.empty() && !allPlayerActorsDead) {
		//  Compute which pixel on the animated pixel map is the
		//  upper left corner of the clip area

		Point16     localOrg;

		//  Compute which pixel on the animated pixel map is the
		//  upper left corner of the clip area

		localOrg.x = animClip.x - animOffset.x;
		localOrg.y = animClip.y - animOffset.y;

		//  Translate the clip area to the coordinate system of
		//  the temporary map, which is relative to the clip.

		animClip.x -= clip.x;
		animClip.y -= clip.y;

		//  Blit the animated pixels into the temp map.

		tempPort.bltPixels(tileDrawMap,
		                   localOrg.x,
		                   localOrg.y,
		                   animClip.x, animClip.y,
		                   animClip.width, animClip.height);
	}

	//  Compute the offset of the local blits (used by drawClipped)

	offset = Point16(clip.x, clip.y);

	//  For each window, both background and float, that overlaps
	//  the clip, draw the window's imagery
	if (userControlsSetup) {
		for (dw = (DecoratedWindow *)G_BASE.bottomWindow();
		        dw;
		        dw = (DecoratedWindow *)dw->prev()) {
			dw->drawClipped(tempPort, offset, clip);
		}
	}
	//  Now, blit the temporary bitmap to the main screen.

	mainPort.setMode(drawModeReplace);
	pointer.hide(mainPort, clip);
	mainPort.bltPixels(tempMap,
	                   0, 0,
	                   clip.x, clip.y, clip.width, clip.height);
	pointer.show(mainPort, clip);
	mainPort.setMode(drawModeMatte);
	RDisposePtr(tempMap.data);
}

void drawFloatingWindows(gPort &port, const Point16 &offset, const Rect16 &clip) {
	DecoratedWindow     *dw;

	//  If a floating window is in the process of being dragged,
	//  then things get a little more complex...

	if (DragBar::update) {
		Rect16          oldExtent,
		                newExtent;

		//  Calculate the new and old position of the window

		oldExtent = DragBar::dragWindow->getExtent();
		newExtent = oldExtent;
		newExtent.x = DragBar::dragPos.x;
		newExtent.y = DragBar::dragPos.y;

		//  Move the window to the new position

		DragBar::dragWindow->setExtent(newExtent);
		DragBar::update = FALSE;

		if (newExtent.overlap(oldExtent)) {
			//  If the positions overlap, update them as a single
			//  unit.

			oldExtent = bound(newExtent, oldExtent);
			updateWindowSection(oldExtent);
		} else {
			//  Otherwise, update new and old positions seperately

			updateWindowSection(newExtent);
			updateWindowSection(oldExtent);
		}
	}

	for (dw = (DecoratedWindow *)G_BASE.bottomWindow();
	        dw;
	        dw = (DecoratedWindow *)dw->prev()) {
		if (!dw->isBackdrop())
			dw->drawClipped(port, offset, clip);
	}
}

void FloatingWindow::toFront(void) {
	gWindow::toFront();
	draw();
}

/* ===================================================================== *
   Misc functions
 * ===================================================================== */

void closeAllFloatingWindows(void) {
}

extern APPFUNC(cmdWindowFunc);

} // end of namespace Saga2
