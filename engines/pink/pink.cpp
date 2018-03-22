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
#include <engines/util.h>
#include <common/debug-channels.h>
#include <video/flic_decoder.h>
#include "engines/pink/objects/module.h"
#include <graphics/surface.h>

namespace Pink {

Pink::PinkEngine::PinkEngine(OSystem *system, const ADGameDescription *desc)
        : Engine(system), _console(nullptr), _rnd("pink"),
          _desc(*desc), _bro(nullptr), _module(nullptr)
{
    debug("PinkEngine constructed");

    DebugMan.addDebugChannel(kPinkDebugGeneral, "general", "General issues");
    DebugMan.addDebugChannel(kPinkDebugLoadingObjects, "loading_objects", "Serializing objects from Orb");
    DebugMan.addDebugChannel(kPinkDebugLoadingResources, "loading_resources", "Loading resources data");
    DebugMan.addDebugChannel(kPinkDebugGraphics, "graphics", "Graphics handling");
    DebugMan.addDebugChannel(kPinkDebugSound, "sound", "Sound processing");
}

Pink::PinkEngine::~PinkEngine() {
    delete _console;
    delete _bro;
    for (uint i = 0; i < _modules.size(); ++i) {
        delete _modules[i];
    }

    DebugMan.clearAllDebugChannels();
}

Common::Error PinkEngine::init() {
    debug("PinkEngine init");

    initGraphics(640, 480);

    _console = new Console(this);

    Common::String orbName{_desc.filesDescriptions[0].fileName};
    Common::String broName{_desc.filesDescriptions[1].fileName};

    if (!broName.empty()){
        _bro = new BroFile();
    }
    else debug("This game doesn't need to use bro");

    if (!_orb.open(orbName) || (_bro && !_bro->open(broName, _orb.getTimestamp()))){
        return Common::kNoGameDataFoundError;
    }

    // TODO load cursor

    _orb.loadGame(this);
    _nextModule = _modules[0]->getName();
    initModule();

    return Common::kNoError;
}

Common::Error Pink::PinkEngine::run() {
    Common::Error error = init();
    if (error.getCode() != Common::kNoError){
        return error;
    }

    Video::FlicDecoder flicDecoder;
    Common::File anim;
    anim.open("WANDRBOY.CEL");
    flicDecoder.loadStream(&anim);
    flicDecoder.start();
    _system->updateScreen();
    const Graphics::Surface *surface = flicDecoder.decodeNextFrame();
    while(!shouldQuit()){
        Common::Event event;
        while(_eventMan->pollEvent(event)){
            switch (event.type){
                case Common::EVENT_QUIT:
                case Common::EVENT_RTL:
                    debug("Quit Event");
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
        surface = flicDecoder.needsUpdate() ? flicDecoder.decodeNextFrame() : surface;
        if (surface) {
            _system->copyRectToScreen(surface->getPixels(), surface->pitch, 0, 0, surface->w, surface->h);
            _system->updateScreen();
        }
        _system->delayMillis(10);
    }

    return Common::kNoError;
}

void PinkEngine::load(Archive &archive) {
    archive.readString();
    archive.readString();
    archive >> _modules;
}

void PinkEngine::initModule() {
    if (_module) {
        assert(_module->getName() != _nextModule);

        //call module function (smth with unloading)

        //check additional field of game(unk_1)
        uint i;
        for (i = 0; i < _modules.size(); ++i) {
            if (_module == _modules[i]){
                break;
            }
        }
        assert(i != _modules.size());

        _modules[i] = new ModuleProxy(_module->getName());

        delete _module;
        _module = nullptr;
    }

    assert(_modules.size() != 0);

    uint i;
    for (i = 0; i < _modules.size(); ++i) {
        assert(dynamic_cast<Module*>(_modules[i]) == 0);
        if (_modules[i]->getName() == _nextModule) {
            loadModule(i);
            break;
        }
    }
    assert(i < _modules.size());

    _module = static_cast<Module*>(_modules[i]);
    _module->init(LoadingNotSave, 0);

}

void PinkEngine::setNextExecutors(const Common::String &nextModule, const Common::String &nextPage) {
    _nextModule = nextModule;
    _nextPage = nextPage;
}

void PinkEngine::loadModule(int index) {
    assert(dynamic_cast<Module*>(_modules[index]) == 0);

    Module *module = new Module(this, _modules[index]->getName());

    _orb.loadObject(module, module->getName());

    delete _modules[index];
    _modules[index] = module;
}

bool PinkEngine::checkValueOfVariable(Common::String &variable, Common::String &value) {
    assert(_variables.contains(variable));
    return _variables[variable] == value;
}

void PinkEngine::setVariable(Common::String &variable, Common::String &value) {
    _variables[variable] = value;
}

}