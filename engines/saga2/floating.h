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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_FLOATING_H
#define SAGA2_FLOATING_H

#include "saga2/fta.h"
#include "saga2/hresmgr.h"
#include "saga2/panel.h"

namespace Saga2 {

//DNode
//	gPanel
//		gControl
class DragControl;
//		gPanelList
//			gWindow
class DecoratedWindow;
class BackWindow;
class FloatingWindow;
struct WindowDecoration;
class  GameObject;

/* ===================================================================== *
   WindowDecoration -- defines border imagery for windows
 * ===================================================================== */

//  BackgroundImage -- for windows which are covered with decorative
//  artwork, this class provides a linked list of all the "artwork
//  panels" which cover the borders of the image.

struct StaticWindow {
	StaticRect extent;
	void *image;
	int16 imageNumber;
};

struct WindowDecoration {
	Rect16          extent;                 // area that image covers
	void            *image;                 // pointer to image data
	int16           imageNumber;            // image resource number

	// default constructor
	WindowDecoration() {
		extent = Rect16(0, 0, 0, 0), image = NULL;
		imageNumber = 0;
	}

	WindowDecoration(const Rect16 &r, int16 num) {
		extent = r;
		image = NULL;
		imageNumber = num;
	}

	WindowDecoration(StaticWindow s) {
		extent = s.extent;
		image = s.image;
		imageNumber = s.imageNumber;
	}

	// this sets the decorations ( for use with the default constructor
	void set(const Rect16 &r, int16 num) {
		extent = r, image = NULL, imageNumber = num;
	}

};


/* ===================================================================== *
   DragBar -- a control which allows the user to drag the window
 * ===================================================================== */

class FloatingWindow;

class DragBar : public gControl {

public:
	static StaticPoint16  dragOffset,             // mouse offset
	                      dragPos;                // new position of window
	static bool     update;                 // true = update window pos
	static FloatingWindow *dragWindow;      // which window to update

	DragBar(gPanelList &, const Rect16 &);

private:
	bool activate(gEventType);
	void deactivate();
	bool pointerHit(gPanelMessage &msg);
	void pointerDrag(gPanelMessage &msg);
	void pointerRelease(gPanelMessage &msg);
};

/* ===================================================================== *
   gButton class: Your basic pushbutton.
 * ===================================================================== */

class gButton : public gControl {
public:

	gButton(gPanelList &list, const Rect16 &box, char *title_, uint16 ident, AppFunc *cmd = NULL) :
		gControl(list, box, title_, ident, cmd) {}
	gButton(gPanelList &list, const Rect16 &box, gPixelMap &img, uint16 ident, AppFunc *cmd = NULL) :
		gControl(list, box, img, ident, cmd) {}

	void draw();                         // redraw the panel.

private:
	bool activate(gEventType why);       // activate the control
	void deactivate();
	bool pointerHit(gPanelMessage &msg);
	void pointerDrag(gPanelMessage &msg);
	void pointerRelease(gPanelMessage &msg);
};

class gPhantomButton : public gButton {
public:
	// Fakes out the gButton into thinking there's no image.
	gPhantomButton(gPanelList &list,
	               const Rect16 &box, uint16 ident, AppFunc *cmd = NULL) :
		gButton(list, box, NULL, ident, cmd) {};
	virtual void draw() {};     // Overrides draw() member of parent, since
	// in this case there's nothing to draw.

};

/* ===================================================================== *
   gImageButton class: a push button with two images (selected & deselected)
 * ===================================================================== */

class gImageButton : public gButton {
protected:
	gPixelMap   *selImage,
	            *deselImage;

public:
	gImageButton(gPanelList &list, const Rect16 &box, gPixelMap &img1, gPixelMap &img2, uint16 ident, AppFunc *cmd = NULL) :
		gButton(list, box, NULL, ident, cmd) {
		selImage = &img1;
		deselImage = &img2;
	}
	gImageButton(gPanelList &list, const Rect16 &box, gPixelMap &img1, gPixelMap &img2, char *title_, uint16 ident, AppFunc *cmd = NULL) :
		gButton(list, box, title_, ident, cmd) {
		selImage = &img1;
		deselImage = &img2;
	}

	void drawClipped(gPort &port, const Point16 &offset, const Rect16 &r);
};

/* ===================================================================== *
   gToggleButton class: Like a push button, but toggles on/off
 * ===================================================================== */

class gToggleButton : public gImageButton {
public:
	gToggleButton(gPanelList &list, const Rect16 &box, gPixelMap &img1, gPixelMap &img2, uint16 ident, AppFunc *cmd) :
		gImageButton(list, box, img1, img2, ident, cmd) {}

private:
	bool activate(gEventType why);       // activate the control
	bool pointerHit(gPanelMessage &msg);
};


/* ===================================================================== *
   LabeledButton class: an image button which displays text
 * ===================================================================== */

class LabeledButton : public gImageButton {
public:
	LabeledButton(gPanelList &list,
	              const Rect16 &box,
	              gPixelMap &img1,
	              gPixelMap &img2,
	              char *buttonLabel,
	              uint16 ident,
	              AppFunc *cmd);

	void drawClipped(gPort &port, const Point16 &offset, const Rect16 &r);
};


/* ===================================================================== *
   DecoratedWindow -- a window with border decorations
 * ===================================================================== */

class DecoratedWindow : public gWindow {
public:
	WindowDecoration        *decorations;
	int16                   numDecorations;

	//  For a future enhancement where different windows have
	//  different animated areas.
//	Rect16                   animatedArea;

	DecoratedWindow(const Rect16 &, uint16, const char saveAs[], AppFunc *cmd = NULL);
	~DecoratedWindow();
	void draw();                         // redraw the window

	//  Redraw the window, but only a small clipped section,
	//  and perhaps drawn onto an off-screen map.
	void drawClipped(
	    gPort         &port,
	    const Point16 &offset,
	    const Rect16  &r);

	//  Attach a list of decorative panels to the window
	void setDecorations(WindowDecoration *, int16, hResContext *);
	void setDecorations(WindowDecoration *, int16, hResContext *, hResID);
	void setDecorations(WindowDecoration *, int16, hResContext *, char, char, char);

	void setDecorations(const StaticWindow *, int16, hResContext *);
	void setDecorations(const StaticWindow *, int16, hResContext *, hResID);
	void setDecorations(const StaticWindow *, int16, hResContext *, char, char, char);


	//  Free up memory used by decorative panels
	void removeDecorations();

	virtual bool isBackdrop();

	//  Update a region of a window, and all floaters which
	//  might be above that window.
	void update(const Rect16 &updateRect);
	void update(const StaticRect &updateRect);
};

/* ===================================================================== *
   BackWindow -- the background window behind everything else
 * ===================================================================== */

class BackWindow : public DecoratedWindow {

	//  Disable the window-to-front
	void toFront();

public:
	BackWindow(const Rect16 &, uint16, AppFunc *cmd = NULL);
	void invalidate(Rect16 *area);
	void invalidate(const StaticRect *area);

	virtual bool isBackdrop();
};

/* ===================================================================== *
   FloatingWindow -- floats on top of background window
 * ===================================================================== */

class FloatingWindow : public DecoratedWindow {

	DragBar     *db;                    // save address of drag bar

	void toFront();

	// original extent before movement
	Point16 origPos;

public:

	// decoration position offset
	Point16 decOffset;

	FloatingWindow(const Rect16 &, uint16, const char saveas[], AppFunc *cmd = NULL);


	//  Redraw the window, but only a small clipped section,
	//  and perhaps drawn onto an off-screen map.
	void drawClipped(
	    gPort         &port,
	    const Point16 &offset,
	    const Rect16  &r);

	// set the extent of the entire window ( including decorations )
	void    setExtent(const Rect16 &);
	const Point16 &getDecOffset() {
		return decOffset;
	}

	bool open();
	void close();
};

/* ===================================================================== *
   Misc functions
 * ===================================================================== */

//  This probably doesn't belong here, but I can't think of a
//  convenient place to put it right now.

void drawCompressedImage(gPort &port, const Point16 pos, void *image);
void drawCompressedImageGhosted(gPort &port, const Point16 pos, void *image);

} // end of namespace Saga2

#endif
