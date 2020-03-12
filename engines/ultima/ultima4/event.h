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

#ifndef ULTIMA4_EVENT_H
#define ULTIMA4_EVENT_H

#include "common/events.h"
#include "common/list.h"
#include "common/str.h"
#include "ultima/ultima4/controller.h"
#include "ultima/ultima4/types.h"
#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Ultima4 {

#define eventHandler (EventHandler::getInstance())

#define U4_UP           '['
#define U4_DOWN         '/'
#define U4_LEFT         ';'
#define U4_RIGHT        '\''
#define U4_BACKSPACE    8
#define U4_TAB          9
#define U4_SPACE        ' '
#define U4_ESC          27
#define U4_ENTER        13
#define U4_ALT          128
#define U4_KEYPAD_ENTER 271
#define U4_META         323
#define U4_FKEY         282
#define U4_RIGHT_SHIFT  303
#define U4_LEFT_SHIFT   304
#define U4_RIGHT_CTRL   305
#define U4_LEFT_CTRL    306
#define U4_RIGHT_ALT    307
#define U4_LEFT_ALT     308
#define U4_RIGHT_META   309
#define U4_LEFT_META    310

extern int eventTimerGranularity;

struct _MouseArea;
class EventHandler;
class TextView;

/**
 * A class for handling keystrokes. 
 */
class KeyHandler {
public:
    virtual ~KeyHandler() {}

    /* Typedefs */
    typedef bool (*Callback)(int, void*);

    /** Additional information to be passed as data param for read buffer key handler */
    typedef struct ReadBuffer {
        int (*_handleBuffer)(Common::String *);
        Common::String *_buffer;
        int _bufferLen;
        int _screenX, _screenY;
    } ReadBuffer;

    /** Additional information to be passed as data param for get choice key handler */
    typedef struct GetChoice {
        Common::String _choices;
        int (*_handleChoice)(int);
    } GetChoice;

    /* Constructors */
    KeyHandler(Callback func, void *data = NULL, bool asyncronous = true);
    
    /* Static functions */    
    static int setKeyRepeat(int delay, int interval);
    static bool globalHandler(int key);    

    /* Static default key handler functions */
    static bool defaultHandler(int key, void *data);
    static bool ignoreKeys(int key, void *data);

    /* Operators */
    bool operator==(Callback cb) const;
    
    /* Member functions */    
    bool handle(int key); 
    virtual bool isKeyIgnored(int key);

protected:
    Callback _handler;
    bool _async;
    void *_data;
};

/**
 * A controller that wraps a keyhander function.  Keyhandlers are
 * deprecated -- please use a controller instead.
 */
class KeyHandlerController : public Controller {
public:
    KeyHandlerController(KeyHandler *handler);
    ~KeyHandlerController();

    virtual bool keyPressed(int key);
    KeyHandler *getKeyHandler();

private:
    KeyHandler *handler;
};

/**
 * A controller to read a Common::String, terminated by the enter key.
 */
class ReadStringController : public WaitableController<Common::String> {
public:
    ReadStringController(int maxlen, int screenX, int screenY, const Common::String &accepted_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890 \n\r\010");
    ReadStringController(int maxlen, TextView *view, const Common::String &accepted_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890 \n\r\010");
    virtual bool keyPressed(int key);

    static Common::String get(int maxlen, int screenX, int screenY, EventHandler *eh = NULL);
    static Common::String get(int maxlen, TextView *view, EventHandler *eh = NULL);
#ifdef IOS
    void setValue(const Common::String &utf8StringValue) {
        value = utf8StringValue;
    }
#endif

protected:
    int maxlen, screenX, screenY;
    TextView *view;
    Common::String accepted;
};

/**
 * A controller to read a integer, terminated by the enter key.
 * Non-numeric keys are ignored.
 */
class ReadIntController : public ReadStringController {
public:
    ReadIntController(int maxlen, int screenX, int screenY);

    static int get(int maxlen, int screenX, int screenY, EventHandler *eh = NULL);
    int getInt() const;
};

/**
 * A controller to read a single key from a provided list.
 */
class ReadChoiceController : public WaitableController<int> {
public:
    ReadChoiceController(const Common::String &choices);
    virtual bool keyPressed(int key);

    static char get(const Common::String &choices, EventHandler *eh = NULL);

protected:
    Common::String choices;
};

/**
 * A controller to read a direction enter with the arrow keys.
 */
class ReadDirController : public WaitableController<Direction> {
public:    
    ReadDirController();
    virtual bool keyPressed(int key);    
};

/**
 * A controller to pause for a given length of time, ignoring all
 * keyboard input.
 */
class WaitController : public Controller {
public:
    WaitController(unsigned int cycles);
    virtual bool keyPressed(int key);
    virtual void timerFired();

    void wait();
    void setCycles(int c);

private:
    unsigned int cycles;
    unsigned int current;
};

/**
 * A class for handling timed events.
 */ 
class TimedEvent {
public:
    /* Typedefs */
    typedef Common::List<TimedEvent*> List;
    typedef void (*Callback)(void *);

    /* Constructors */
    TimedEvent(Callback callback, int interval, void *data = NULL);

    /* Member functions */
    Callback getCallback() const;
    void *getData();
    void tick();
    
    /* Properties */
protected:    
    Callback callback;
    void *data;
    int interval;
    int current;
};

#if defined(IOS)
#ifndef __OBJC__
typedef void *TimedManagerHelper;
typedef void *UIEvent;
#else
@class TimedManagerHelper;
@class UIEvent;
#endif
#endif


/**
 * A class for managing timed events
 */ 
class TimedEventMgr {
public:
    /* Typedefs */
    typedef TimedEvent::List List;    

    /* Constructors */
    TimedEventMgr(int baseInterval);
    ~TimedEventMgr();

    /* Static functions */
    static unsigned int callback(unsigned int interval, void *param);

    /* Member functions */
    bool isLocked() const;      /**< Returns true if the event list is locked (in use) */    

    void add(TimedEvent::Callback callback, int interval, void *data = NULL);
    List::iterator remove(List::iterator i);
    void remove(TimedEvent* event);
    void remove(TimedEvent::Callback callback, void *data = NULL);
    void tick();
    void stop();
    void start();
    
    void reset(unsigned int interval);     /**< Re-initializes the event manager to a new base interval */
#if defined(IOS)
    bool hasActiveTimer() const;
#endif

private:
    void lock();                /**< Locks the event list */
    void unlock();              /**< Unlocks the event list */

    /* Properties */
protected:
    /* Static properties */
    static unsigned int instances;

    void *id;
    int baseInterval;
    bool locked;
    List events;
    List deferredRemovals;
#if defined(IOS)
    TimedManagerHelper *m_helper;
#endif
};

typedef void(*updateScreenCallback)(void);
/**
 * A class for handling game events. 
 */
class EventHandler {
public:    
    /* Typedefs */
    typedef Common::List<_MouseArea *> MouseAreaList;    

    /* Constructors */
    EventHandler();    

    /* Static functions */    
    static EventHandler *getInstance();
    static void sleep(unsigned int usec);
    static void wait_msecs(unsigned int msecs);
    static void wait_cycles(unsigned int cycles);
    static void setControllerDone(bool exit = true);
    static bool getControllerDone();
    static void end();
    static bool timerQueueEmpty();

    /* Member functions */
    TimedEventMgr* getTimer();

    /* Event functions */    
    void run();
    void setScreenUpdate(void (*updateScreen)(void));
#if defined(IOS)
    void handleEvent(UIEvent *);
    static void controllerStopped_helper();
    updateScreenCallback screenCallback() { return updateScreen; }
#endif

    /* Controller functions */
    Controller *pushController(Controller *c);
    Controller *popController();
    Controller *getController() const;
    void setController(Controller *c);

    /* Key handler functions */
    void pushKeyHandler(KeyHandler kh);
    void popKeyHandler();
    KeyHandler *getKeyHandler() const;
    void setKeyHandler(KeyHandler kh);

    /* Mouse area functions */
    void pushMouseAreaSet(_MouseArea *mouseAreas);
    void popMouseAreaSet();
    _MouseArea* getMouseAreaSet() const;
    _MouseArea* mouseAreaForPoint(int x, int y);

protected:    
    static bool controllerDone;
    static bool ended;
    TimedEventMgr timer;
    Std::vector<Controller *> controllers;
    MouseAreaList mouseAreaSets;
    updateScreenCallback updateScreen;

private:
    static EventHandler *instance;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
