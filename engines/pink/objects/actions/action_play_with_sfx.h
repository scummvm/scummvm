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

#ifndef PINK_ACTION_PLAY_WITH_SFX_H
#define PINK_ACTION_PLAY_WITH_SFX_H

#include "pink/objects/actions/action_play.h"

namespace Pink {

class ActionSfx;

class ActionPlayWithSfx : public ActionPlay {
public:
    virtual ~ActionPlayWithSfx();
    virtual void deserialize(Archive &archive);
    virtual void toConsole();
    virtual void update();

protected:
    virtual void onStart();

private:
    void updateSound();
    uint32 _isLoop;
    Array<ActionSfx*> _sfxArray;
};

class Sound;
class GamePage;

class ActionSfx : public Object {
public:
    ActionSfx();
    virtual ~ActionSfx();
    virtual void deserialize(Archive &archive);
    virtual void toConsole();

    void play(GamePage *page);
    uint32 getFrame();
    void end();

private:
    Sound *_sound;
    Common::String _sfxName;
    uint32 _volume;
    uint32 _frame;
};

} // End of namespace Pink

#endif
