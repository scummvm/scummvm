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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "asylum/speech.h"
#include "asylum/actor.h"
#include "asylum/config.h"
#include "asylum/text.h"

namespace Asylum {

Speech::Speech(Scene *scene): _scene(scene) {
    _tick = _scene->vm()->getTick();
    _soundResIdx = 0;
    _textResIdx = 0;
}

Speech::~Speech() {
	// TODO Auto-generated destructor stub
}

int32 Speech::play(int32 speechIdx) {
    int32 soundResIdx = 0;

    switch (_scene->worldstats()->actorType) {
    case kMax: {
        int32 soundResIdx2 = speechIdx;
        int32 textResIdx = speechIdx;

        if (speechIdx >= 259) {
            soundResIdx2 -= 9;
            textResIdx -= 9;
        }

        soundResIdx = soundResIdx2 - 0x7FFD0000;
        // setup the right index for sound and text
        setPlayerSpeech(soundResIdx, textResIdx - 0x7FFFFFAD);
    }
        break;
    // TODO: check if actor type is right for the following cases
    case kSarah: {
        soundResIdx = speechIdx - 0x7FFBF879;
        setPlayerSpeech(soundResIdx, speechIdx - 0x7FFFFDB6);
    }
        break;
    case kCyclops: {
        soundResIdx = speechIdx - 0x7FFBF7DC;
        setPlayerSpeech(soundResIdx, speechIdx - 0x7FFFFD19);
    }
        break;
    case kAztec: {
        soundResIdx = speechIdx - 0x7FFBF746;
        setPlayerSpeech(soundResIdx, speechIdx - 0x7FFFFC83);
    }
        break;
    default:
        soundResIdx = 0;
        break;
    }

    return soundResIdx;
}

void Speech::setPlayerSpeech(int32 soundResIdx, int32 textResIdx) {
    if (soundResIdx) {
        if (_scene->vm()->sound()->isPlaying(soundResIdx)) {
            _scene->vm()->sound()->stopSound(soundResIdx);
        }
    }

    _soundResIdx = soundResIdx;
    _textResIdx = textResIdx;

    prepareSpeech();
}

void Speech::prepareSpeech() {
    //int32 startTick = _scene->vm()->getTick();

    if (_soundResIdx) {
        if (!_scene->vm()->sound()->isPlaying(_soundResIdx)/* || _tick && startTick >= _tick*/) {
            processSpeech();
        }

        if (Config.showEncounterSubtitles) {
            int16 check = 0;
            /*Common::Point *pt;
            _scene->getActorPosition(_scene->getActor(), pt);
            
            check = pt->y < 240;
            check = pt->y >= 240;*/
            int32 posY = ((check - 1) & 0x118) + 40;

            _scene->vm()->text()->loadFont(_scene->getResourcePack(), _scene->worldstats()->commonRes.font3);
            _scene->vm()->text()->drawText(20, posY, _textDataPos);

            _scene->vm()->text()->loadFont(_scene->getResourcePack(), _scene->worldstats()->commonRes.font1);
            _scene->vm()->text()->drawText(20, posY, _textData);
        }
    }
}

void Speech::processSpeech() {
    char * txt;

    _tick = 0;

    txt = _scene->vm()->text()->getResText(_textResIdx);

    if (*(txt + strlen((const char *)txt) - 2) == 1) {
        _textResIdx = 0;
        _textData = 0;
        _textDataPos = 0;
    } else {
        if (*txt == '{') {
            _textData = txt + 3;
            _textDataPos = 0;
            
            _scene->vm()->text()->loadFont(_scene->getResourcePack(), _scene->worldstats()->commonRes.font1);
            _scene->vm()->sound()->playSpeech(_soundResIdx);
        } else {
            _textData = 0;
            _textDataPos = txt;
            
            if (*txt == '/') {
                _textDataPos = txt + 2;
            }
            
            _scene->vm()->text()->loadFont(_scene->getResourcePack(), _scene->worldstats()->commonRes.font3);
            _scene->vm()->sound()->playSpeech(_soundResIdx);
        }
    }
}

} // end of namespace Asylum
