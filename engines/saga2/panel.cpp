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

#include "common/events.h"

#include "saga2/saga2.h"
#include "saga2/panel.h"
#include "saga2/fontlib.h"
#include "saga2/floating.h"
#include "saga2/display.h"
#include "saga2/gbevel.h"

namespace Saga2 {

//extern vDisplayPage   *drawPage;
extern gMousePointer pointer;           // the actual pointer
extern char iniFile[];
extern vDisplayPage protoPage;

//  Function to enable/disable user interface keys
extern bool enableUIKeys(bool enabled);

/* ======================================================================= *
   global dispatcher base
 * ======================================================================= */

gToolBase           G_BASE;
gDisplayPort        *globalPort;
gFont               *mainFont;

//  UI locking feature, currently kludged it could get better later.
int         lockUINest = 0;

/* ======================================================================= *
   gPanel member functions
 * ======================================================================= */

gPanel::gPanel(gWindow &win, const Rect16 &box, AppFunc *cmd)
	: window(win), extent(box), command(cmd) {
	enabled = 1;
	ghosted = 0;
	selected = 0;
	imageLabel = 0;
}

gPanel::gPanel(gPanelList &list, const Rect16 &box,
               const char *newTitle, uint16 ident, AppFunc *cmd)
	: window(list.window) {
	title = newTitle;
	extent = box;
	enabled = 1;
	ghosted = 0;
	selected = 0;
	imageLabel = 0;
	command = cmd;
	id = ident;
}

gPanel::gPanel(gPanelList &list, const Rect16 &box,
               gPixelMap &pic, uint16 ident, AppFunc *cmd)
	: window(list.window) {
	title = (char *)&pic;
	extent = box;
	enabled = 1;
	ghosted = 0;
	selected = 0;
	imageLabel = 1;
	command = cmd;
	id = ident;
}

gPanel::gPanel(gPanelList &list, const StaticRect &box,
               const char *newTitle, uint16 ident, AppFunc *cmd)
	: window(list.window) {
	title = newTitle;
	extent = Rect16(box);
	enabled = 1;
	ghosted = 0;
	selected = 0;
	imageLabel = 0;
	command = cmd;
	id = ident;
}

//  Dummy virtual functions

gPanel::~gPanel() {
	if (this == G_BASE.mousePanel)
		G_BASE.mousePanel = NULL;
	if (this == G_BASE.activePanel)
		G_BASE.activePanel = NULL;

}
void gPanel::draw(void) {}
void gPanel::drawClipped(gPort &, const Point16 &, const Rect16 &) {}
void gPanel::pointerMove(gPanelMessage &) {}
bool gPanel::pointerHit(gPanelMessage &) {
	return false;
}
bool gPanel::pointerRHit(gPanelMessage &) {
	return false;
}
void gPanel::pointerDrag(gPanelMessage &) {}
void gPanel::pointerRelease(gPanelMessage &) {}
bool gPanel::keyStroke(gPanelMessage &) {
	return false;
}
void gPanel::timerTick(gPanelMessage &) {}
void gPanel::onMouseHintDelay(void) {}

void gPanel::enable(bool abled) {
	enabled = abled ? 1 : 0;
}

void gPanel::select(uint16 sel) {
	selected = sel ? 1 : 0;
}

void gPanel::ghost(bool b) {
	ghosted = b ? 1 : 0;
}

bool gPanel::isActive(void) {
	return (this == G_BASE.activePanel);
}

void gPanel::notify(enum gEventType type, int32 value) {
	gEvent          ev;

	ev.panel = this;
	ev.eventType = type;
	ev.value = value;
	ev.mouse.x = G_BASE.pickPos.x - extent.x;
	ev.mouse.y = G_BASE.pickPos.y - extent.y;
	ev.window = &window;

	if (command) command(ev);
	else if (this != &window) window.notify(ev);
}

bool gPanel::activate(gEventType) {
	return false;
}

void gPanel::deactivate(void) {
	if (isActive()) G_BASE.activePanel = NULL;
}

void gPanel::makeActive(void) {
	G_BASE.setActive(this);
}

void gPanel::invalidate(Rect16 *) {
	assert(displayEnabled());
	window.update(extent);
}


void gPanel::drawTitle(enum text_positions placement) {
	gPort           &port = window.windowPort;
	Rect16          r = extent;
	const gPixelMap *img = nullptr;

	if (title == NULL)
		return;

	if (imageLabel) {
		img = (const gPixelMap *)title;
		r.width = img->size.x;
		r.height = img->size.y;
	} else {
		r.width = TextWidth(mainFont, title, -1, textStyleUnderBar);
		r.height = mainFont->height;
	}

	switch (placement) {
	case textPosLeft:
		r.x -= r.width + 2;
		r.y += (extent.height - r.height) / 2 + 1;
		break;

	case textPosRight:
		r.x += extent.width + 3;
		r.y += (extent.height - r.height) / 2 + 1;
		break;

	case textPosHigh:
		r.x += (extent.width - r.width) / 2;
		r.y -= r.height + 1;
		break;

	case textPosLow:
		r.x += (extent.width - r.width) / 2;
		r.y += extent.height + 2;
		break;

	default:
		r.x += (extent.width - r.width) / 2;
		r.y += (extent.height - r.height) / 2;
		break;
	}

	SAVE_GPORT_STATE(port);                  // save pen color, etc.

	if (imageLabel) {
		port.setIndirectColor(blackPen);     // pen color black
		port.setMode(drawModeColor);         // draw as glyph
		port.bltPixels(*img, 0, 0, r.x, r.y, r.width, r.height);
	} else {
		port.setMode(drawModeMatte);         // draw as glyph
		port.setIndirectColor(blackPen);     // pen color black
		port.setStyle(textStyleUnderBar);    // set style to do underbars
		port.moveTo(r.x, r.y);           // move to new text pos

		pointer.hide(*globalPort, r);        // hide the pointer
		port.drawText(title, -1);            // draw the text
		pointer.show(*globalPort, r);        // hide the pointer
	}
}

gPanel *gPanel::hitTest(const Point16 &p) {
	return enabled && !ghosted && extent.ptInside(p) ? this : NULL;
}

gPanel *gPanel::keyTest(int16) {
	return NULL;
}

/* ===================================================================== *
   gPanelList class: A context for holding panels.
 * ===================================================================== */

//  Constructor which is called from window subclass

gPanelList::gPanelList(gWindow &win, const Rect16 &box, char *newTitle,
                       uint16 ident, AppFunc *cmd)
	: gPanel(win, box, cmd) {
	title = newTitle;
	id = ident;
}

//  Constructor for standalone panels..

gPanelList::gPanelList(gPanelList &list)
	: gPanel(list, list.window.getExtent(), NULL, 0, NULL) {
	window.contents.push_back(this);
}

gPanelList::~gPanelList() {
	removeControls();
	window.contents.remove(this);
}

void gPanelList::removeControls(void) {
	gPanel *ctl;

	//  Delete all sub-panels.
	while (contents.size()) {
		ctl = contents.front();
		contents.remove(ctl);
		delete ctl;
	}
}

//  enable/disable gPanelList and all it's children
void gPanelList::enable(bool abled) {
	gPanel::enable(abled);
}

void gPanelList::invalidate(Rect16 *) {
	gPanel *ctl;
	Rect16 invArea;

	assert(displayEnabled());

	if (displayEnabled())
		if (contents.size()) {
			ctl = contents.back();
			invArea = ctl->getExtent();

			for (Common::List<gPanel *>::iterator it = contents.reverse_begin(); it != contents.end(); --it) {
				ctl = *it;
				invArea = bound(invArea, ctl->getExtent());
			}
			window.update(invArea);
		}
}

void gPanelList::draw(void) {
	gPanel *ctl;

	if (displayEnabled())
		if (enabled) {
			for (Common::List<gPanel *>::iterator it = contents.reverse_begin(); it != contents.end(); --it) {
				ctl = *it;
				if (ctl->getEnabled())
					ctl->draw();
			}
		}
}

void gPanelList::drawClipped(
    gPort           &port,
    const Point16   &offset,
    const Rect16    &r) {
	gPanel          *ctl;
	Point16         tmpOffset = offset - Point16(extent.x, extent.y);
	Rect16          tmpR = r - Point16(extent.x, extent.y);

	if (displayEnabled())
		if (enabled) {
			for (Common::List<gPanel *>::iterator it = contents.reverse_begin(); it != contents.end(); --it) {
				ctl = *it;
				if (ctl->getEnabled())
					ctl->drawClipped(port, tmpOffset, tmpR);
			}
		}
}

gPanel *gPanelList::hitTest(const Point16 &p) {
	gPanel        *ctl;
	gPanel        *result;

	if (enabled && !ghosted) {
		for (Common::List<gPanel *>::iterator it = contents.begin(); it != contents.end(); ++it) {
			ctl = *it;
			if ((result = ctl->hitTest(p)) != NULL)
				return result;
		}
	}
	return NULL;
}

gPanel *gPanelList::keyTest(int16 key) {
	gPanel          *ctl;
	gPanel          *result;

	if (enabled && !ghosted) {
		for (Common::List<gPanel *>::iterator it = contents.reverse_begin(); it != contents.end(); --it) {
			ctl = *it;
			if ((result = ctl->keyTest(key)) != NULL)
				return result;
		}
	}

	return NULL;
}

/* ===================================================================== *
   gWindow class: A panel list plus a drawing port.
 * ===================================================================== */

//  gWindow static variables

int             gWindow::dragMode;              // current dragging mode
Rect16          gWindow::dragExtent;            // dragging extent
Point16         gWindow::dragOffset;            // offset to window origin

gWindow::gWindow(const Rect16 &box, uint16 ident, const char saveName[], AppFunc *cmd)
	: gPanelList(*this, box, NULL, ident, cmd)
	  //, saver(WIIFF_POS|WIIFS_NORMAL|WIIFE_ONEXIT,iniFile,saveName,box,this)
{
	openFlag = false;
//	pointerImage = &arrowPtr;
//	pointerOffset = Point16( 0, 0 );

	//  Set up the window feature bits

//	windowFeatures = features;

	//  Set up the window's gPort

	windowPort.setDisplayPage(globalPort->displayPage);
	windowPort.setFont(mainFont);
	windowPort.setPenMap(globalPort->penMap);

	/*  if (windowFeatures & windowBackSaved)   // backsave data under window
	    {
	        backSave = NEW_UI gBackSave( box );
	        // rem: if backsave fails, then what?
	    }
	    else backSave = NULL;
	*/
	//  Set the window position.

	setPos(Point16(box.x, box.y));
}

gWindow::~gWindow() {
//	gControl     *ctl;

	if (isOpen()) close();

	//  Delete all sub-panels.

//	while ( (ctl = (gControl *)contents.remHead()) != NULL )
//		delete ctl;

//	delete backSave;
}

bool gWindow::open(void) {
	if (isOpen()) return true;

	//  Send a "pointer-leave" message to mouse panel.

	G_BASE.leavePanel();
	G_BASE.windowList.push_front(this);
	G_BASE.activeWindow = this;
	G_BASE.setActive(NULL);

//	pointer.hide();
//	if (backSave) backSave->save( *globalPort );
//	pointer.setImage( *pointerImage, pointerOffset.x, pointerOffset.y );
//	pointer.show();

	openFlag = true;

	draw();
	return true;
}

void gWindow::close(void) {
	//saver.onExit(this);
	if (!isOpen()) return;

	//  If any panels on this window are active, then deactivate them.
	if (G_BASE.activePanel && G_BASE.activePanel->getWindow() == this)
		G_BASE.activePanel->deactivate();

	//  Don't close a window that is being dragged (should never happen,
	//  but just in case).
	if (DragBar::dragWindow == (FloatingWindow *)this)
		return;

	openFlag = false;

	//  remove this window from the window list.

	G_BASE.windowList.remove(this);

	G_BASE.mouseWindow = G_BASE.activeWindow = G_BASE.windowList.front();
	G_BASE.mousePanel = G_BASE.activePanel = NULL;
}

//  Move the window to the front...

void gWindow::toFront(void) {            // re-order the windows
	if (!isOpen()) return;

	G_BASE.windowList.remove(this);
	G_BASE.windowList.push_front(this);

	G_BASE.activePanel = NULL;
	G_BASE.activeWindow = this;

	//  redraw the window
	update(extent);
}

bool gWindow::isModal(void) {
	return false;
}

void gWindow::setPos(Point16 pos) {
	Rect16  newClip;

	extent.x = pos.x;
	extent.y = pos.y;

//	int16            titleHeight = mainFont->height + 5;

	//  We also need to set up the window's port in a similar fashion.

	windowPort.origin.x = extent.x;
	windowPort.origin.y = extent.y;

	//  set port's clip
	newClip = intersect(extent, g_vm->_mainPort.clip);
	newClip.x -= extent.x;
	newClip.y -= extent.y;
	windowPort.setClip(newClip);
	//saver.onMove(this);

//	if (backSave) backSave->setPos( pos );
}

void gWindow::setExtent(const Rect16 &r) {
	extent.width = r.width;
	extent.height = r.height;

	//saver.onSize(this);
	setPos(Point16(r.x, r.y));
}

//  insert window into window list
void gWindow::insert(void) {
	G_BASE.windowList.push_front(this);
}


//  REM: Need to either adjuct coords when we draw OR
//  redefine the address of the pixel map.

void gWindow::deactivate(void) {
	selected = 0;
	gPanel::deactivate();
}

bool gWindow::activate(gEventType why) {
	if (why == gEventMouseDown) {           // momentarily depress
		selected = 1;
		notify(why, 0);                      // notify App of successful hit
		return true;
	}
	return false;
}

void gWindow::pointerMove(gPanelMessage &) {
	notify(gEventMouseMove, 0);
}

bool gWindow::pointerHit(gPanelMessage &) {
	activate(gEventMouseDown);
	return true;
}

void gWindow::pointerDrag(gPanelMessage &) {
	if (selected) {
		notify(gEventMouseDrag, 0);
	}
}

void gWindow::pointerRelease(gPanelMessage &) {
	if (selected) notify(gEventMouseUp, 0);   // notify App of successful hit
	deactivate();
}

void gWindow::draw(void) {
	if (displayEnabled())
		gPanelList::draw();
}

void gWindow::drawClipped(
    gPort           &port,
    const Point16   &offset,
    const Rect16    &r) {
	if (displayEnabled())
		gPanelList::drawClipped(port, offset, r);
}

void gWindow::enable(bool abled) {
	gPanel::enable(abled);
	draw();
}

void gWindow::select(uint16 sel) {
	gPanel::select(sel);
	draw();
}

/*
void gWindow::setPointer( gPixelMap &map, int x, int y )
{
    pointerImage = &map;
    pointerOffset.x = x;
    pointerOffset.y = y;

    if (this == G_BASE.activeWindow)
    {
        pointer.hide();
        pointer.setImage( *pointerImage, pointerOffset.x, pointerOffset.y );
        pointer.show();
    }
}
*/

/* ===================================================================== *
   gControl class: The basis for buttons and other controls.
 * ===================================================================== */

gControl::gControl(gPanelList &list, const Rect16 &box, const char *title_, uint16 ident,
                   AppFunc *cmd) : gPanel(list, box, title_, ident, cmd) {
	accelKey = 0;

	//  Add control to the window's control list.

	list.contents.push_back(this);
}

gControl::gControl(gPanelList &list, const Rect16 &box, gPixelMap &img, uint16 ident,
                   AppFunc *cmd) : gPanel(list, box, img, ident, cmd) {
	accelKey = 0;

	//  Add control to the window's control list.

	list.contents.push_back(this);
}

gControl::~gControl() {
	window.contents.remove(this);
}

gControl::gControl(gPanelList &list, const StaticRect &box, const char *title_, uint16 ident,
                   AppFunc *cmd) : gPanel(list, box, title_, ident, cmd) {
	accelKey = 0;

	//  Add control to the window's control list.

	list.contents.push_back(this);
}

void gControl::enable(bool abled) {
	if (!abled != !getEnabled()) {  //  Use '!' to insure boolean-ness
		gPanel::enable(abled);
		invalidate();
	}
}

void gControl::select(uint16 sel) {
	if (!sel != !isSelected()) {    //  Use '!' to insure boolean-ness
		gPanel::select(sel);
		invalidate();
	}
}

void gControl::ghost(bool sel) {
	if (!sel != !isGhosted()) {     //  Use '!' to insure boolean-ness
		gPanel::ghost(sel);
		invalidate();
	}
}

gPanel *gControl::keyTest(int16 key) {
	return accelKey == key ? this : NULL;
}

//  For many controls, the only drawing routine we need is the
//  "clipped" one, and the normal draw routine just calls
//  drawClipped with the main port.

void gControl::draw(void) {
	pointer.hide(window.windowPort, extent);
	if (displayEnabled())
		drawClipped(*globalPort,
		            Point16(-window.extent.x, -window.extent.y),
		            window.extent);
	pointer.show(window.windowPort, extent);
}

/* ===================================================================== *
   gGenericControl class: A generic button that notifies everything
 * ===================================================================== */

gGenericControl::gGenericControl(gPanelList &list, const Rect16 &box,
                                 uint16 ident, AppFunc *cmd)
	: gControl(list, box, NULL, ident, cmd) {
	dblClickFlag = false;
}

bool gGenericControl::activate(gEventType) {
	selected = 1;
	return true;
}

void gGenericControl::deactivate(void) {
	selected = 0;
	gPanel::deactivate();
}

void gGenericControl::pointerMove(gPanelMessage &msg) {
	notify(gEventMouseMove, (msg.pointerEnter ? enter : 0) | (msg.pointerLeave ? leave : 0));
}

bool gGenericControl::pointerHit(gPanelMessage &msg) {
	if (msg.rightButton)
		notify(gEventRMouseDown, 0);
	else if (msg.doubleClick && !dblClickFlag) {
		dblClickFlag = true;
		notify(gEventDoubleClick, 0);
	} else {
		dblClickFlag = false;
		notify(gEventMouseDown, 0);
	}

	return true;
}

void gGenericControl::pointerDrag(gPanelMessage &) {
	notify(gEventMouseDrag, 0);
}

void gGenericControl::pointerRelease(gPanelMessage &) {
	notify(gEventMouseUp, 0);
	deactivate();
}

//  Generic control has no rendering code.
void gGenericControl::draw(void) {
}

/* ===================================================================== *
   class gToolBase: Global dispatcher for events
 * ===================================================================== */

void gToolBase::setActive(gPanel *ctl) {
	if (activePanel && activePanel == ctl)  return;
	if (activePanel) activePanel->deactivate();
	if (ctl == NULL || ctl->activate(gEventNone)) activePanel = ctl;
}

void gToolBase::handleMouse(Common::Event &event, uint32 time) {
	gWindow         *w = activeWindow;
	gPanel          *ctl,
	                *pickPanel = NULL;
	static gMouseState prevState;
	static int32    lastClickTime = 0x8000;
	static Point16  lastClickPos;


	// Emulate mouse state for now
	switch (event.type) {
	case Common::EVENT_LBUTTONDOWN:
		_curMouseState.left = true;
		break;
	case Common::EVENT_RBUTTONDOWN:
		_curMouseState.right = true;
		break;
	case Common::EVENT_LBUTTONUP:
		_curMouseState.left = false;
		break;
	case Common::EVENT_RBUTTONUP:
		_curMouseState.right = false;
		break;
	case Common::EVENT_MOUSEMOVE:
		_curMouseState.pos.x = event.mouse.x;
		_curMouseState.pos.y = event.mouse.y;
		break;
	default:
		break;
	}

	//  Do nothing if UI locked.
	if (lockUINest > 0)
		return;

#if CURSOR_CYCLING
	if (_curMouseState.right) {
		cycleCursor();
		return;
	}
#endif

	//  Code for "Tool tip delay"
	if (prevState.pos != _curMouseState.pos
	        &&  prevState.left != _curMouseState.left
	        &&  prevState.right != _curMouseState.right) {
		lastMouseMoveTime = msg.timeStamp;
		if (mouseHintSet)
			setMouseTextF(NULL);
	}

	//  If there is no active window, then do nothing.

	if (w == NULL) {
		prevState = _curMouseState;
		return;
	}

	//  Set up the pick position relative to the window

	if (activePanel) {
		pickPos.x = _curMouseState.pos.x - activePanel->window.extent.x;
		pickPos.y = _curMouseState.pos.y - activePanel->window.extent.y;
	} else {
		pickPos.x = _curMouseState.pos.x - w->extent.x;
		pickPos.y = _curMouseState.pos.y - w->extent.y;
	}

	//  Fill in the message to be sent to the various panels

	msg.pickAbsPos  = pickPos;
	msg.leftButton  = _curMouseState.left ? 1 : 0;
	msg.rightButton = _curMouseState.right ? 1 : 0;
	msg.pointerEnter = 0;
	msg.pointerLeave = 0;
	msg.doubleClick = 0;
	msg.timeStamp = time;

	if (((_curMouseState.left  && !leftDrag)            // if left button hit
	        || (_curMouseState.right && !rightDrag))      // or right button hit
	        && activePanel != NULL) {           // and a panel is active
		//  Then we have a button hit event. If the button hit
		//  is occuring outside the panel, then it should be
		//  deselected.

		if (activePanel->extent.ptInside(pickPos) == false)
			activePanel->deactivate();
	}

	if (prevState.pos == _curMouseState.pos) ;          // don't do anything if same pos
	else if (activePanel) {                 // if control active
		mousePanel = activePanel;           // assume mouse over active panel

		if (leftDrag || rightDrag) {
			setMsg(msg, activePanel);        // set up gPanelMessage
			activePanel->pointerDrag(msg);  // send panel a mouse movement
		} else {
			setMsg(msg, activePanel);        // set up gPanelMessage
			activePanel->pointerMove(msg);  // send panel a mouse movement
		}
	}

	if (!activePanel /* && !ms.right */) {
		//  If the point is within the window

		for (Common::List<gWindow *>::iterator it = windowList.begin(); it != windowList.end(); ++it) {
			w = *it;
			if (w->extent.ptInside(_curMouseState.pos) || w->isModal()) {
				//  Set up the pick position relative to the window

				pickPos.x = _curMouseState.pos.x - w->extent.x;
				pickPos.y = _curMouseState.pos.y - w->extent.y;

				if ((ctl = w->hitTest(pickPos)) != NULL)
					pickPanel = ctl;
				else
					pickPanel = w;

				break;
			}
		}

		if (w == NULL) {
			prevState = _curMouseState;
			return;
		}

		mouseWindow = w;

		//  If the mouse is not over the control any more, tell it so.

		if (mousePanel && mousePanel != pickPanel) {
			if (&mousePanel->window != w) {
				//  Temporarily adjust pickPos to be relative to the old panel's window
				//  instead of the new panel's window.
				pickPos.x = _curMouseState.pos.x - mousePanel->window.extent.x;
				pickPos.y = _curMouseState.pos.y - mousePanel->window.extent.y;

				setMsgQ(msg, mousePanel);        // set up gPanelMessage

				pickPos.x = _curMouseState.pos.x - w->extent.x;
				pickPos.y = _curMouseState.pos.y - w->extent.y;
			} else {
				setMsgQ(msg, mousePanel);        // set up gPanelMessage
			}
//			msg.pickPos.x  = pickPos.x - mousePanel->extent.x;
//			msg.pickPos.y  = pickPos.y - mousePanel->extent.y;
			msg.inPanel     = 0;
			msg.pointerEnter = 0;
			msg.pointerLeave = 1;

			mousePanel->pointerMove(msg);

		}

		//  If the mouse is over a new control, make that the current
		//  mouse control.

		if (pickPanel) {
			setMsg(msg, pickPanel);          // set up gPanelMessage
//			msg.pickPos.x  = pickPos.x - pickPanel->extent.x;
//			msg.pickPos.y  = pickPos.y - pickPanel->extent.y;
			msg.leftButton  = _curMouseState.left ? 1 : 0;
//			msg.inPanel        = pickPanel->extent.ptInside(pickPos);
			msg.pointerEnter = (mousePanel == pickPanel) ? 0 : 1;
			msg.pointerLeave = 0;

			mousePanel = pickPanel;
			mousePanel->pointerMove(msg);
		} else
			mousePanel = NULL;
	}

	//  Fix up flags because earlier code may have changed them

	msg.pointerEnter = 0;
	msg.pointerLeave = 0;

	//  Send appropriate button-press messages to the panels

	if (prevState.left  != _curMouseState.left         // if buttons changed state
	        || prevState.right != _curMouseState.right) {

		//  If both buttons were previously up, then a mouse
		//  hit must have occured.

		if (prevState.left == 0 && prevState.right == 0) {

			//  Check for mouse double-click. Check to see that
			//  the elapsed time from the last click is less than
			//  1/3 of a second, and that the mouse ptr hasn't moved
			//  very much.

			if (((uint32)(msg.timeStamp - lastClickTime) < 333)
			        ||  _curMouseState.left > 1
			        ||  _curMouseState.right > 1) {
				Point16 diff = lastClickPos - _curMouseState.pos;

				if (abs(diff.x) + abs(diff.y) < 6)
					msg.doubleClick = 1;
			}

			//  Record the last mouse time and position for
			//  future double-click checks.

			lastClickTime = msg.timeStamp;
			lastClickPos = _curMouseState.pos;

			if (mousePanel) {               // if over a control
				setMsgQ(msg, mousePanel);        // set up gPanelMessage
//				msg.pickPos.x = pickPos.x - mousePanel->extent.x;
//				msg.pickPos.y = pickPos.y - mousePanel->extent.y;
				msg.inPanel     = 1;

				if (activeWindow && activeWindow->isModal()) {
					mouseWindow = activeWindow;
				} else if (mouseWindow == NULL) {
					mouseWindow = &mousePanel->window;
				}

				if (mouseWindow && mouseWindow != activeWindow) {
					msg.pickAbsPos = pickPos;
					//  Re-order the windows.
					mouseWindow->toFront();
				}
				// send it a hit message
				if (mousePanel->pointerHit(msg)) {
					activePanel = mousePanel;
					if (_curMouseState.left)
						leftDrag = true;
					else
						rightDrag = true;
				}
			}
		} else if ((leftDrag && _curMouseState.left == false)  // check for release
		           || (rightDrag && _curMouseState.right == false)) {
			if (activePanel && mousePanel) {            // if a control is active
				setMsg(msg, mousePanel);    // send it a release message
				mousePanel->pointerRelease(msg);
			}
			leftDrag = rightDrag = false;
		}
	}

	prevState = _curMouseState;
}

void gToolBase::leavePanel(void) {
	msg.timeStamp = g_system->getMillis();

	if (mousePanel) {
		msg.inPanel     = 0;
		msg.pointerEnter = 0;
		msg.pointerLeave = 1;

		mousePanel->pointerMove(msg);
		mousePanel = NULL;
	}

	if (activePanel) activePanel->deactivate();
}

void gToolBase::handleKeyStroke(Common::Event &event) {
	gWindow         *w = activeWindow;
	gPanel          *ctl;

	uint16 key = event.kbd.ascii; // FIXME
	uint16 qualifier = 0;

	if (event.kbd.flags & Common::KBD_SHIFT)
		qualifier |= qualifierShift;

	if (event.kbd.flags & Common::KBD_CTRL)
		qualifier |= qualifierControl;

	if (event.kbd.flags & Common::KBD_ALT)
		qualifier |= qualifierAlt;

	msg.pickAbsPos  = pickPos;
	msg.pointerEnter = 0;
	msg.pointerLeave = 0;
	msg.key = ((key & 0xFF) != 0) ? key & 0xff : (key >> 8) + 0x80;
	msg.qualifier = qualifier;
	msg.timeStamp = g_system->getMillis();

	if (activePanel) {                      // send keystroke to active panel
		setMsg(msg, activePanel);            // set up gPanelMessage
		if (activePanel->keyStroke(msg))
			return;
	}

	//  Now, search the contents of the window for a control with
	//  the correct accelerator key

	if (w) {
		uint8   k = ((key & 0xFF) != 0) ? key & 0xff : (key >> 8) + 0x80;
		//uint8 k = key & 0xff;

		if (k != 0) {
			k = toupper(k);

			if ((ctl = w->keyTest(k)) != NULL) {
				if (activePanel == ctl) return;
				if (activePanel) activePanel->deactivate();
				if (ctl->activate(gEventKeyDown)) {
					activePanel = ctl;
					return;
				}
			}
		}

		//  Try sending the message to the window

		if (w->keyStroke(msg)) return;

		// else send the message to the app.

		if (key & 0xff)
			key &= 0xff;
		w->notify(gEventKeyDown, (qualifier << 16) | key);
	}
}

void gToolBase::handleTimerTick(int32 tick) {
	msg.pickAbsPos  = pickPos;
	msg.pointerEnter = 0;
	msg.pointerLeave = 0;
	msg.timeStamp = tick;

	if (activePanel) {                      // send keystroke to active panel
		setMsg(msg, activePanel);            // set up gPanelMessage
		activePanel->timerTick(msg);
	} else if (mousePanel) {
		if (mousePanel->wantMousePoll) {
			setMsg(msg, mousePanel);         // set up gPanelMessage
			mousePanel->pointerMove(msg);
		} else if (!mouseHintSet
		           && ((uint32)(tick - lastMouseMoveTime) > 500)) {
			mousePanel->onMouseHintDelay();
		}
	}
}

void HandleTimerTick(long tick) {
	static int32        lastTick;

	if (tick - lastTick > 1) {
		lastTick = tick;
		G_BASE.handleTimerTick(tick);
	}
}

/* ===================================================================== *
   Code to initialize the panel system
 * ===================================================================== */

void initPanels(gDisplayPort &port) {
	port.setDisplayPage(&protoPage);
	globalPort = &port;
	mainFont = &Helv11Font;
}

void cleanupPanels(void) {
}

int16 leftButtonState(void) {
	return G_BASE.msg.leftButton;
}

int16 rightButtonState(void) {
	return G_BASE.msg.rightButton;
}

void LockUI(bool state) {
	if (state == true) {
		if (lockUINest <= 0) {
			pointer.hide();
			enableUIKeys(false);
			G_BASE.setActive(NULL);
		}
		lockUINest++;
	} else {
		lockUINest--;
		assert(lockUINest >= 0);
		if (lockUINest <= 0) {
			enableUIKeys(true);
			pointer.show();
		}
	}
}

void dumpGBASE(char *msg) {
}

} // end of namespace Saga2
