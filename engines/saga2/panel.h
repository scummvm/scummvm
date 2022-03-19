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

void cycleCursor();

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

void HandleTimerTick(long newTick);
void EventLoop(bool &running, bool modal = false);

/* ===================================================================== *
                                Constants
 * ===================================================================== */

enum gEventType {
	gEventNone = 0,                         // no event occurred
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
	enum gEventType eventType;              // type of event that occurred
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
	Rect16          _extent;                 // rectangular bounds of the control
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
	virtual void onMouseHintDelay();

	void notify(enum gEventType, int32 value);
	void notify(gEvent &ev) {
		if (command) command(ev);
	}
	void drawTitle(enum text_positions placement);


public:
	bool isActive();                     // true if we are active panel
	virtual bool activate(gEventType why);  // activate the control
	virtual void deactivate();       // deactivate the control
	virtual void draw();                 // redraw the panel.
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
	gWindow *getWindow() {
		return &window;
	}
	void makeActive();
	Rect16 getExtent() {
		return _extent;
	}
	bool isSelected() {
		return selected != 0;
	}
	bool isGhosted() {
		return ghosted != 0;
	}
	bool    getEnabled() const {
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

	gPanelMessage() {
		leftButton = 0;
		rightButton = 0;
		inPanel = 0;
		pointerEnter = 0;
		pointerLeave = 0;
		doubleClick = 0;
		key = 0;
		qualifier = 0;
		timeStamp = 0;
	}
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
	void removeControls();

public:
	void invalidate(Rect16 *area = nullptr);
	void draw();                         // redraw the controls
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
	void deactivate();

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
	static StaticRect  dragExtent;             // dragging extent
	static StaticPoint16 dragOffset;             // offset to window origin

	void shadow();

public:
	void setExtent(const Rect16 &);          // set window position and size
	Rect16 getExtent() {
		return _extent;    // set window position and size
	}
protected:
	void setPos(Point16 pos);                // set window position
	void insert();                      // insert window into window list
	virtual void toFront();              // re-order the windows

public:
	bool isOpen() {
		return openFlag;    // true if window is visible
	}
	void draw();                         // redraw the panel.
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

	virtual bool open();
	virtual void close();
	virtual bool isModal();

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
	gPanelList *_list;

	gControl(gPanelList &, const Rect16 &, const char *, uint16, AppFunc *cmd = NULL);
	gControl(gPanelList &, const Rect16 &, gPixelMap &, uint16, AppFunc *cmd = NULL);

	gControl(gPanelList &, const StaticRect &, const char *, uint16, AppFunc *cmd = NULL);
	~gControl();                            // destructor

	gPanel *keyTest(int16 key);

	void enable(bool abled);                 // enable the control
	void select(uint16 sel);               // selecte the control
	void ghost(bool ghosted);
//	virtual void newValue( void );

	void draw();                         // redraw the control.
};

/* ===================================================================== *
   gGenericControl class: A generic button that notifies everything
 * ===================================================================== */

class gGenericControl : public gControl {
	bool dblClickFlag;

public:
	gGenericControl(gPanelList &, const Rect16 &, uint16, AppFunc *cmd = NULL);

	//  Disable double click for next mouse click
	void disableDblClick() {
		dblClickFlag = true;
	}

	enum    controlValue {
		enter = (1 << 0),
		leave = (1 << 1)
	};

protected:
	bool activate(gEventType why);       // activate the control
	void deactivate();

	void pointerMove(gPanelMessage &msg);
	bool pointerHit(gPanelMessage &msg);
	void pointerDrag(gPanelMessage &msg);
	void pointerRelease(gPanelMessage &msg);

	void draw();                         // redraw the control.
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
	friend int16        leftButtonState();
	friend int16        rightButtonState();
	friend void     StageModeCleanup();
	friend void     TileModeCleanup();
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

	gToolBase() {
		mouseWindow = nullptr;
		activeWindow = nullptr;
		mousePanel = nullptr;
		activePanel = nullptr;
		leftDrag = 0;
		rightDrag = 0;
		lastMouseMoveTime = 0;
	}

private:
	void setMsgQ(gPanelMessage &msg_, gPanel *panel) {
		if (panel == &panel->window)
			msg_.pickPos = pickPos;
		else {
			msg.pickPos.x = (int16)(pickPos.x - panel->_extent.x);
			msg.pickPos.y = (int16)(pickPos.y - panel->_extent.y);
		}
	}

	void setMsg(gPanelMessage &msg_, gPanel *panel) {
		setMsgQ(msg_, panel);
		msg.inPanel = (msg_.pickPos.x >= 0
		               && msg_.pickPos.y >= 0
		               && msg_.pickPos.x < panel->_extent.width
		               && msg_.pickPos.y < panel->_extent.height);
		//          panel->extent.ptInside( pickPos );
	}

public:
	void setActive(gPanel *newActive);
	void leavePanel();               // we're changing windows
public:
	void handleMouse(Common::Event &event, uint32 time);
	void handleKeyStroke(Common::Event &event);
	void handleTimerTick(int32 tick);
	Common::List<gWindow *>::iterator topWindowIterator() {
		return windowList.end();
	}
	Common::List<gWindow *>::iterator bottomWindowIterator() {
		return windowList.reverse_begin();
	}
	gWindow *topWindow() {
		return windowList.front();
	}
	gWindow *bottomWindow() {
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
void cleanupPanels();

//void writeHelpLine( char *msg, ... );

int16 leftButtonState();
int16 rightButtonState();

//  Kludge structure to contain both a mouse event and a time stamp
struct MouseExtState {
	gMouseState st;
	int32       time;
};

} // end of namespace Saga2

#endif
