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

#ifndef SAGA2_PANEL_H
#define SAGA2_PANEL_H

#include "saga2/input.h"
#include "saga2/vdraw.h"

namespace Common {
struct Event;
}

namespace Saga2 {
//  Fix problem with DOS's encroachment on name space that should
//  rightfully belong to the application programmer.

#ifdef enable
#undef enable
#undef disable
#endif

/* ===================================================================== *
                          Cursor Cycling Stuff
 * ===================================================================== */

#if DINO
#define CURSOR_CYCLING 1
#endif
#if CURSOR_CYCLING
void cycleCursor();
#endif

/* ===================================================================== *
                          Forward declarations
 * ===================================================================== */

class gPanel;
class gPanelList;
class gPanelMessage;
class gWindow;

/* ===================================================================== *
                      Input dispatching functions
 * ===================================================================== */

void HandleMouse(gMouseState &mouse);
void HandleKeyStroke(int key, int qual);
void HandleTimerTick(long newTick);
void EventLoop(bool &running, bool modal = false);

/* ===================================================================== *
                                Constants
 * ===================================================================== */

enum gEventType {
	gEventNone = 0,                         // no event occured
	gEventMouseDown,                        // left button pressed
	gEventMouseUp,                          // left button released
	gEventRMouseDown,                       // right button pressed
	gEventRMouseUp,                         // right button released
	gEventMouseMove,                        // mouse moved
	gEventMouseDrag,                        // mouse dragged
	gEventMouseOutside,                     // mouse click outside of window
	gEventKeyDown,                          // keystroke

	gEventNewValue,                         // object had new value
	gEventDoubleClick,                      // double-clicked on object
	gEventAltValue,                         // for multi-function objects

	gEventLast
};

/* ===================================================================== *
   gEvent: returns input events to the user
 * ===================================================================== */

struct gEvent {
	gPanel          *panel;                 // where event came from
	enum gEventType eventType;              // type of event that occured
	Point16         mouse;                  // mouse position
	int32           value;                  // new value of control
	gWindow         *window;                // active window
};

typedef void        AppFunc(gEvent &);

#ifndef __WATCOMC__
#define APPFUNCDECL __cdecl
#else
#define APPFUNCDECL
#endif

#define APPFUNC(a)  void a ( gEvent &ev )
#define APPFUNCV(a) void a ( gEvent & )

//  Note: panels in this system are not, in general, resizeable

/* ===================================================================== *
   gPanel class: The basic user interface element
 * ===================================================================== */

class gPanel {
	friend class    gToolBase;
	friend class    gWindow;

	AppFunc         *command;               // application function
protected:
	gWindow         &window;                // window this belongs to
	Rect16          extent;                 // rectangular bounds of the control
	const char      *title;                 // title of the panel
	byte             enabled,            // allows disabling the panel
	                selected,           // some panels have a selected state
	                imageLabel,         // button label is image, not text
	                ghosted,            // button is dimmed
	                wantMousePoll;      // send mousemoves even if mouse not moving!

	// window constructor
	gPanel(gWindow &, const Rect16 &, AppFunc *cmd);

public:
	uint32          id;                     // panel id number
	void            *userData;              // data for this panel

	// constructor
	gPanel(gPanelList &, const Rect16 &, const char *, uint16, AppFunc *cmd = NULL);
	gPanel(gPanelList &, const Rect16 &, gPixelMap &, uint16, AppFunc *cmd = NULL);

	gPanel(gPanelList &, const StaticRect &, const char *, uint16, AppFunc *cmd = NULL);
	virtual ~gPanel();                      // destructor

	virtual gPanel *hitTest(const Point16 &p);
	virtual gPanel *keyTest(int16 key);

protected:
	virtual void pointerMove(gPanelMessage &msg);
	virtual bool pointerHit(gPanelMessage &msg);
	virtual bool pointerRHit(gPanelMessage &msg);
	virtual void pointerDrag(gPanelMessage &msg);
	virtual void pointerRelease(gPanelMessage &msg);
	virtual bool keyStroke(gPanelMessage &msg);
	virtual void timerTick(gPanelMessage &msg);
	virtual void onMouseHintDelay(void);

	void notify(enum gEventType, int32 value);
	void notify(gEvent &ev) {
		if (command) command(ev);
	}
	void drawTitle(enum text_positions placement);


public:
	bool isActive(void);                     // true if we are active panel
	virtual bool activate(gEventType why);  // activate the control
	virtual void deactivate(void);       // deactivate the control
	virtual void draw(void);                 // redraw the panel.
	virtual void enable(bool abled);
	virtual void select(uint16 selected);
	virtual void ghost(bool ghosted);
	virtual void invalidate(Rect16 *area = nullptr);
	virtual void setMousePoll(bool abled) {
		wantMousePoll = abled ? 1 : 0;
	}

	//  Redraw the panel, but only a small clipped section,
	//  and perhaps drawn onto an off-screen map.
	virtual void drawClipped(
	    gPort         &port,
	    const Point16 &offset,
	    const Rect16  &r);

//	void setCommand( AppFunc *func ) { command = func; }
	gWindow *getWindow(void) {
		return &window;
	}
	void makeActive(void);
	Rect16 getExtent(void) {
		return extent;
	}
	bool isSelected(void) {
		return selected != 0;
	}
	bool isGhosted(void) {
		return ghosted != 0;
	}
	bool    getEnabled(void) const {
		return (bool) enabled;
	}
	void    show(bool shown = true, bool inval = true) {
		enable(shown);
		if (inval)
			invalidate();
	}

	void moveToFront(gPanelList &l);
	void moveToBack(gPanelList &l);
};

/* ===================================================================== *
   gPanelMessage class: How user input is distributes to panels
 * ===================================================================== */

class gPanelMessage {
public:
	Point16         pickPos,                // mouse position relative to panel
	                pickAbsPos;             // mouse position relative to display
	byte             leftButton,         // left button state
	                rightButton,        // right button state
	                inPanel,            // whether mouse is currently in panel
	                pointerEnter,       // set when pointer enters panel
	                pointerLeave,       // set when pointer leaves panel
	                doubleClick;        // set when double click detected

	//  For keyboard input

	uint16          key,                    // keystroke from keyboard
	                qualifier;              // qualifier from keyboard

	uint32          timeStamp;              // time of message
};

/* ===================================================================== *
   gPanelList class: A list of panels
 * ===================================================================== */

class gControl;

class gPanelList : public gPanel {

	friend class    gControl;
	friend class    gToolBase;

	friend void gPanel::moveToFront(gPanelList &l);
	friend void gPanel::moveToBack(gPanelList &l);

protected:

	Common::List<gPanel *> contents;               // list of panels

	gPanelList(gWindow &, const Rect16 &, char *, uint16, AppFunc *cmd = NULL);

public:

	gPanelList(gPanelList &);
	~gPanelList();

	gPanel *hitTest(const Point16 &p);
	gPanel *keyTest(int16 key);
	void removeControls(void);

public:
	void invalidate(Rect16 *area = nullptr);
	void draw(void);                         // redraw the controls
	void drawClipped(
	    gPort         &port,
	    const Point16 &offset,
	    const Rect16  &r);

	void enable(bool abled);                 // enable list and all children
//	void setPointer( gPixelMap &map, int x, int y );
};

inline void gPanel::moveToFront(gPanelList &l) {
	l.contents.remove(this);
	l.contents.push_front(this);
}

inline void gPanel::moveToBack(gPanelList &l) {
	l.contents.remove(this);
	l.contents.push_back(this);
}

/* ===================================================================== *
   gWindow class: A context for holding panels.
 * ===================================================================== */

/*
enum windowFeatures {
    windowTitleBar  = (1<<0),               // window has a title bar
    windowCanDrag   = (1<<1),               // window is draggable
//  windowVisible   = (1<<2),               // window is visible
//  windowBackSaved = (1<<3),               // backsave data under window
//  windowCloseBox  = (1<<4),               // create a close box
//  windowNoBorder  = (1<<5),               // window with no border
};
*/

class gWindow : public gPanelList {

	friend class    gControl;
	friend class    gPanel;
	friend class    gToolBase;

public:
	gDisplayPort    windowPort;

	gWindow(const Rect16 &, uint16, const char saveName[], AppFunc *cmd = NULL);
	~gWindow();

	operator gPort() {
		return windowPort;
	}
	void postEvent(gEvent &ev) {
		gPanel::notify(ev);
	}

protected:
	bool            openFlag;               // true if window open.

	//gWindowWinInfoInINIFile saver;

private:
	bool activate(gEventType why);       // activate the control
	void deactivate(void);

	void pointerMove(gPanelMessage &msg);
	bool pointerHit(gPanelMessage &msg);
	void pointerDrag(gPanelMessage &msg);
	void pointerRelease(gPanelMessage &msg);

	//  Dragging modes for window -- private and static!

	enum drag_modes {
		dragNone = 0,
		dragPosition
	};

	static int      dragMode;               // current dragging mode
	static Rect16   dragExtent;             // dragging extent
	static Point16  dragOffset;             // offset to window origin

	void shadow(void);

public:
	void setExtent(const Rect16 &);          // set window position and size
	Rect16 getExtent(void) {
		return extent;    // set window position and size
	}
protected:
	void setPos(Point16 pos);                // set window position
	void insert(void);                      // insert window into window list
	virtual void toFront(void);              // re-order the windows

public:
	bool isOpen(void) {
		return openFlag;    // true if window is visible
	}
	void draw(void);                         // redraw the panel.
	void drawClipped(
	    gPort         &port,
	    const Point16 &offset,
	    const Rect16  &r);

	//  Redraw the window, but only a small clipped section,
	//  and perhaps drawn onto an off-screen map.
//	void drawClipped(
//		gPort         &port,
//		const Point16 &offset,
//		const Rect16  &r );

	void enable(bool abled);
	void select(uint16 sel);               // activate the window

	virtual bool open(void);
	virtual void close(void);
	virtual bool isModal(void);

	//  Update a region of a window, and all floaters which
	//  might be above that window.
	virtual void update(const Rect16 &updateRect) = 0;

//	void setPointer( gPixelMap &map, int x, int y );
};

/* ===================================================================== *
   gControl class: The basis for buttons and other controls.
 * ===================================================================== */

class gControl : public gPanel {
public:
	uint8               accelKey;

	gControl(gPanelList &, const Rect16 &, const char *, uint16, AppFunc *cmd = NULL);
	gControl(gPanelList &, const Rect16 &, gPixelMap &, uint16, AppFunc *cmd = NULL);

	gControl(gPanelList &, const StaticRect &, const char *, uint16, AppFunc *cmd = NULL);
	~gControl();                            // destructor

	gPanel *keyTest(int16 key);

	void enable(bool abled);                 // enable the control
	void select(uint16 sel);               // selecte the control
	void ghost(bool ghosted);
//	virtual void newValue( void );

	void draw(void);                         // redraw the control.
};

/* ===================================================================== *
   gGenericControl class: A generic button that notifies everything
 * ===================================================================== */

class gGenericControl : public gControl {
	bool dblClickFlag;

public:
	gGenericControl(gPanelList &, const Rect16 &, uint16, AppFunc *cmd = NULL);

	//  Disable double click for next mouse click
	void disableDblClick(void) {
		dblClickFlag = true;
	}

	enum    controlValue {
		enter = (1 << 0),
		leave = (1 << 1)
	};

protected:
	bool activate(gEventType why);       // activate the control
	void deactivate(void);

	void pointerMove(gPanelMessage &msg);
	bool pointerHit(gPanelMessage &msg);
	void pointerDrag(gPanelMessage &msg);
	void pointerRelease(gPanelMessage &msg);

	void draw(void);                         // redraw the control.
};


/* ===================================================================== *
   gToolBase class: The primary dispatcher for user interface events
 * ===================================================================== */

void     setMouseTextF(char *format, ...);
void     LockUI(bool state);

class gToolBase {
	friend class    gWindow;
	friend class    gPanel;
	friend void     EventLoop(bool &running);
	friend int16        leftButtonState(void);
	friend int16        rightButtonState(void);
	friend void     StageModeCleanup(void);
	friend void     TileModeCleanup(void);
	friend void dumpGBASE(char *msg);

	// windows

	Common::List<gWindow *> windowList;     // list of windows
	gWindow         *mouseWindow,           // window mouse is in
	                *activeWindow;          // current active window
	gPanel          *mousePanel,            // panel that mouse is in
	                *activePanel;           // panel that has input focus
	Rect16          dragRect;               // dragging rectangle for windows
	Point16         pickPos;                // mouse pos relative to panel
	uint8           leftDrag,               // left-button dragging
	                rightDrag;              // right button dragging
	gPanelMessage   msg;                    // message that we send out

	int32           lastMouseMoveTime;      // time of last mouse move

	gMouseState _curMouseState;

public:
	bool            mouseHintSet;           // true if mouse hint is up.

private:
	void setMsgQ(gPanelMessage &msg_, gPanel *panel) {
		if (panel == &panel->window)
			msg_.pickPos = pickPos;
		else {
			msg.pickPos.x = (int16)(pickPos.x - panel->extent.x);
			msg.pickPos.y = (int16)(pickPos.y - panel->extent.y);
		}
	}

	void setMsg(gPanelMessage &msg_, gPanel *panel) {
		setMsgQ(msg_, panel);
		msg.inPanel = (msg_.pickPos.x >= 0
		               && msg_.pickPos.y >= 0
		               && msg_.pickPos.x < panel->extent.width
		               && msg_.pickPos.y < panel->extent.height);
		//          panel->extent.ptInside( pickPos );
	}

public:
	void setActive(gPanel *newActive);
	void leavePanel(void);               // we're changing windows
public:
	void handleMouse(Common::Event &event, uint32 time);
	void handleKeyStroke(Common::Event &event);
	void handleTimerTick(int32 tick);
	Common::List<gWindow *>::iterator topWindowIterator(void) {
		return windowList.end();
	}
	Common::List<gWindow *>::iterator bottomWindowIterator(void) {
		return windowList.reverse_begin();
	}
	gWindow *topWindow(void) {
		return windowList.front();
	}
	gWindow *bottomWindow(void) {
		return windowList.back();
	}
	bool isMousePanel(gPanel *p) {
		return (mousePanel != NULL) ? (p == mousePanel) : (p == topWindow());
	}
};

/* ===================================================================== *
   Application functions to call for event handling
 * ===================================================================== */

void EventLoop(bool &running);
void initPanels(gDisplayPort &port);
void cleanupPanels(void);

//void writeHelpLine( char *msg, ... );

int16 leftButtonState(void);
int16 rightButtonState(void);

//  Kludge structure to contain both a mouse event and a time stamp
struct MouseExtState {
	gMouseState st;
	int32       time;
};

} // end of namespace Saga2

#endif
