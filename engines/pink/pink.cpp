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

#include "pink.h"
#include "console.h"
#include <audio/mixer.h>
#include <engines/util.h>

namespace Pink {

Pink::PinkEngine::PinkEngine(OSystem *system, const ADGameDescription *desc)
        : Engine(system), _rnd("pink"), _desc(*desc),
          _bro(nullptr)
{
    /* TODO
     *  setup debug channels
     *
     */
}

Pink::PinkEngine::~PinkEngine() {
    delete _console;
    delete _bro;
}

Common::Error PinkEngine::init() {
    initGraphics(640, 480);

    _console = new Console(this);

    if (_desc.filesDescriptions[1].fileName){
        _bro = new BroFile();
    }

    Common::String orbName = _desc.filesDescriptions[0].fileName;
    Common::String broName = _desc.filesDescriptions[1].fileName;

    if (!_orb.open(orbName) || (_bro && !_bro->open(broName, _orb.getTimestamp()))){
        return Common::kNoGameDataFoundError;
    }

    return Common::kNoError;
}

Common::Error Pink::PinkEngine::run() {
    Common::Error error = init();
    if (error.getCode() != Common::kNoError){
        return error;
    }

    while(!shouldQuit()){
        Common::Event event;
        while(_eventMan->pollEvent(event)){
            switch (event.type){
                case Common::EVENT_QUIT:
                case Common::EVENT_RTL:

                    return Common::kNoError;
                case Common::EVENT_MOUSEMOVE:

                    break;
                case Common::EVENT_LBUTTONDOWN:

                    break;

                    // don't know why it is used in orginal
                case Common::EVENT_LBUTTONUP:
                case Common::EVENT_RBUTTONDOWN:
                case Common::EVENT_KEYDOWN:
                    break;
                default:
                    break;
            }
        }
        //update();
        g_system->updateScreen();
        g_system->delayMillis(10);
    }


    return Common::kNoError;
}

}