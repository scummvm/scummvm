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
 */

#include "common/system.h"
#include "gamos/gamos.h"

namespace Gamos {

MidiMusic::MidiMusic() {
    MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_PREFER_GM);

    _driver = MidiDriver::createMidi(dev);
    _driver->open();
    _driver->sendGMReset();
}

MidiMusic::~MidiMusic() {
    g_system->getTimerManager()->removeTimerProc(_timerProc);

    if (_driver) {
        _driver->stopAllNotes(true);
        _driver->close();
        delete _driver;
    }
}

void MidiMusic::stopMusic() {
    g_system->getTimerManager()->removeTimerProc(_timerProc);

    if (_mutex.lock()) {
        _driver->stopAllNotes(true);

        _mutex.unlock();
    }
}

bool MidiMusic::playMusic(Common::Array<byte> *midiData) {
    stopMusic();

    if (!midiData || midiData->size() <= 4)
        return false;

    if (!_mutex.lock())
        return false;

    _pMidiData.clear();
    _pMidiData.swap(*midiData);

    _dataStart = 4;
    _dataPos = _dataStart;
    _midiDelayTicks = 1;
    _midiDelayCount = 1;

    if (_pMidiData[_dataPos] == 0xf8) {
        _dataPos++;
        midi2low();
        _dataPos++;
    }

    g_system->getTimerManager()->installTimerProc(_timerProc, 10 * 1000, this, "Gamos::Music");

    _mutex.unlock();
    return true;
}

void MidiMusic::update() {
    while (true) {
        if (_dataPos >= _pMidiData.size())
            return;

        uint8 b = _pMidiData[_dataPos];

        if (b > 0x7f) {
            /* only if new event type then update _midiOp */
            _midiOp = b;
            _dataPos++;
        }

        bool doDelay = true;

        if (_midiOp == 0xf0 || _midiOp == 0xf7) {
            int16 skipLen = midi2low();
            if (skipLen >= 0)
                _dataPos += skipLen;
        } else if (_midiOp == 0xff) {
            if (_midiDelayTicks != -1) {
                _midiDelayCount = _midiDelayTicks;
                _dataPos = _dataStart;
            } else {
                g_system->getTimerManager()->removeTimerProc(_timerProc);
                _driver->stopAllNotes(true);
            }
            break;
        } else {
            uint8 param1 = _pMidiData[_dataPos];
            uint8 param2 = 0;
            _dataPos++;

            bool doSend = true;

            uint8 cmd = _midiOp & 0xf0;
            if (cmd != MidiDriver_BASE::MIDI_COMMAND_PROGRAM_CHANGE &&
                cmd != MidiDriver_BASE::MIDI_COMMAND_CHANNEL_AFTERTOUCH) {
                if (cmd == MidiDriver_BASE::MIDI_COMMAND_NOTE_OFF ||
                    cmd == MidiDriver_BASE::MIDI_COMMAND_NOTE_ON) {
                    if (_midiMute)
                        doSend = false;
                }

                b = _pMidiData[_dataPos];
                _dataPos++;

                doDelay = (b & 0x80) == 0;
                param2 = b & 0x7f;
            }

            if (doSend)
                _driver->send(_midiOp, param1, param2);
        }

        if (doDelay) {
            int16 ln = midi2low();
            if (ln > 0) {
                _midiTimeStamp += ln * 10;
                if (g_system->getMillis() < _midiTimeStamp)
                    break;
            }
        }
    }
}

int16 MidiMusic::midi2low() {
    if (_dataPos >= _pMidiData.size())
        return -1;

    int16 dat = _pMidiData[_dataPos];
    _dataPos++;

    if (dat & 0x80) {
        if (_dataPos >= _pMidiData.size())
            return -1;

        dat &= 0x7f;
        dat |= _pMidiData[_dataPos] << 7;

        _dataPos++;
    }
    return dat;
}


void MidiMusic::_timerProc(void *data) {
    if (!data)
        return;

    MidiMusic *_this = (MidiMusic *)data;

    if (_this->_midiDelayCount != 0) {
        _this->_midiDelayCount--;
        if (_this->_midiDelayCount != 0)
            return;

        _this->_midiTimeStamp = g_system->getMillis();
    }

    if (_this->_midiTimeStamp <= g_system->getMillis() && _this->_mutex.lock()) {
        _this->update();
        _this->_mutex.unlock();
    }
}

};
