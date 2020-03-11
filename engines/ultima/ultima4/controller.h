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

#ifndef ULTIMA4_CONTROLLER_H
#define ULTIMA4_CONTROLLER_H

namespace Ultima {
namespace Ultima4 {

/**
 * A generic controller base class.  Controllers are classes that
 * contain the logic for responding to external events (e.g. keyboard,
 * mouse, timers).
 */
class Controller {
public:
    Controller(int timerInterval = 1);
    virtual ~Controller();

    /* methods for interacting with event manager */
    virtual bool isCombatController() const { return false; }
    bool notifyKeyPressed(int key);
    int getTimerInterval();
    static void timerCallback(void *data);

    /* control methods subclasses may want to override */
    virtual bool keyPressed(int key) = 0;
    virtual void timerFired();

private:
    int timerInterval;
};

// helper functions for the waitable controller; they just avoid
// having eventhandler dependencies in this header file
void Controller_startWait();
void Controller_endWait();

/**
 * Class template for controllers that can be "waited for".
 * Subclasses should set the value variable and call doneWaiting when
 * the controller has completed.
 */
template<class T>
class WaitableController : public Controller {
public:
    WaitableController() : exitWhenDone(false) {}

    virtual T getValue() {
        return value;
    }

    virtual T waitFor() {
        exitWhenDone = true;
        Controller_startWait();
        return getValue();
    }

protected:
    T value;
    void doneWaiting() {
        if (exitWhenDone)
            Controller_endWait();
    }

private:
    bool exitWhenDone;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
