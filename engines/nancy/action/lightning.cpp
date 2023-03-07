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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/nancy/graphics.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/nancy.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/action/lightning.h"

#include "common/stream.h"
#include "common/random.h"

namespace Nancy {
namespace Action {

void editPalette(byte *colors, uint percent) {
    float alpha = (float) percent / 100;

    for (int i = 0; i < 256 * 3; ++i) {
        uint16 origColor = colors[i];
        colors[i] = MIN<uint16>(alpha * origColor + origColor, 255);
    }
}

LightningOn::~LightningOn() {
    for (byte *palette : _viewportObjOriginalPalettes) {
        delete[] palette;
    }
}

void LightningOn::readData(Common::SeekableReadStream &stream) {
    int16 distance = stream.readSint16LE();
    uint16 pulseTime = stream.readUint16LE();
    int16 rgbPercent = stream.readSint16LE();

    int16 midpoint;
    float delta;

    // Calculate the min & max power of the lightning
    midpoint = (rgbPercent - (distance * 5));
    delta = 0.4 * midpoint;

    _minRGBPercent = MAX<uint16>(0, midpoint - delta);
    _maxRGBPercent = MIN<uint16>(rgbPercent, midpoint + delta);

    // Calculate the min & max delay between lightning strikes
    midpoint = 13000 - (pulseTime * 500);
    delta = 1.5 * midpoint;

    _minInterPulseDelay = MAX<int16>(500, midpoint - delta);
    _maxInterPulseDelay = MIN<int16>(13000, midpoint + delta);

    // Calculate the min & max length of the lightning strikes
    // _minPulseLength is always 5 due to an oversight in the original code
    _maxPulseLength = pulseTime * 10;

    // Calculate the min & max delay between end of lightning and start of thunder sound
    midpoint = distance * 400;
    delta = midpoint * 0.4;

    _minSoundStartDelay = MAX<int16>(250, midpoint - delta);
    _maxSoundStartDelay = midpoint + delta; // No minimum value, probably a bug

    stream.skip(0x4); // paletteStart, paletteSize
}

void LightningOn::execute() {
    if (_state == kBegin) {
        g_nancy->_graphicsManager->grabViewportObjects(_viewportObjs);

        for (RenderObject *obj : _viewportObjs) {
            if (!obj) {
                continue;
            }

            _viewportObjOriginalPalettes.push_back(new byte[256 * 3]);
            obj->grabPalette(_viewportObjOriginalPalettes.back());
        }

        _state = kRun;
    }

    switch (_lightningState) {
    case kStartPulse:
        _nextStateTime = g_nancy->getTotalPlayTime() + g_nancy->_randomSource->getRandomNumberRngSigned(_minPulseLength, _maxPulseLength);
        handleThunder();
        handlePulse(true);
        _lightningState = kPulse;
        break;
    case kPulse:
        if (g_nancy->getTotalPlayTime() > _nextStateTime) {

            _nextStateTime = g_nancy->getTotalPlayTime() + g_nancy->_randomSource->getRandomNumberRngSigned(_minInterPulseDelay, _maxInterPulseDelay);

            _lightningState = kThunder;
            
            if (!g_nancy->_sound->isSoundPlaying("TH1")) {
                _nextSoundToPlay = 0;
                _nextSoundTime0 = g_nancy->getTotalPlayTime() + g_nancy->_randomSource->getRandomNumberRngSigned(_minSoundStartDelay, _maxSoundStartDelay);
            } else if (!g_nancy->_sound->isSoundPlaying("TH2")) {
                _nextSoundToPlay = 1;
                _nextSoundTime1 = g_nancy->getTotalPlayTime() + g_nancy->_randomSource->getRandomNumberRngSigned(_minSoundStartDelay, _maxSoundStartDelay);
            } else {
                _nextSoundToPlay = -1;
            }

            handlePulse(false);
        }
        
        handleThunder();
        break;
    case kThunder:
        if (g_nancy->getTotalPlayTime() > _nextStateTime) {
            _lightningState = kStartPulse;
        }

        handleThunder();
        break;
    }
}

void LightningOn::handlePulse(bool on) {
    for (uint i = 0; i < _viewportObjs.size(); ++i) {
        RenderObject *obj = _viewportObjs[i];

        if (!obj) {
            continue;
        }

        if (on) {
            byte newPalette[256 * 3];
            obj->grabPalette(newPalette);
            editPalette(newPalette, g_nancy->_randomSource->getRandomNumberRngSigned(_minRGBPercent, _maxRGBPercent));
            obj->setPalette(newPalette);
        } else {
            obj->setPalette(_viewportObjOriginalPalettes[i]);
        }
        
    }
}

void LightningOn::handleThunder() {
    if (_nextSoundToPlay == 0) {
        if (g_nancy->getTotalPlayTime() > _nextSoundTime0) {
            g_nancy->_sound->playSound("TH1");
        }
    } else if (_nextSoundToPlay == 1) {
        if (g_nancy->getTotalPlayTime() > _nextSoundTime1) {
            g_nancy->_sound->playSound("TH2");
        }
    }
}

} // End of namespace Action
} // End of namespace Nancy
