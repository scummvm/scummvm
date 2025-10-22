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

 #define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include "gamos/gamos.h"

namespace Gamos {

bool MoviePlayer::init(Common::File *file, uint32 offset, GamosEngine *gamos) {
    _gamos = gamos;
    _screen = _gamos->_screen;
    _messageProc = &_gamos->_messageProc;


    _loopCount = 1;
    _pos = Common::Point();
    _midiBufferSize = 0;
    _soundBufferSize = 0;
    _paletteBufferSize = 0;
    _bufferSize = 0;
    _packedBufferSize = 0;
    _frameTime = 0;
    _loopPoint = 0;
    _midiBuffer.clear();
    _soundBuffer.clear();
    _paletteBuffer.clear();
    _buffer.clear();
    _packedBuffer.clear();
    _midiStarted = false;
    _soundPlaying = false;
    _frameSize = Common::Point(_screen->w, _screen->h);

    _gamos->stopMidi();
    _gamos->stopMCI();

    _file = file;
    return _file->seek(offset, SEEK_SET);
}

bool MoviePlayer::deinit() {
    if (_soundPlaying)
        _gamos->stopSounds();

    _gamos->stopMidi();
    _gamos->stopMCI();

    _gamos->setPaletteCurrentGS();

    _file = nullptr;
    return true;
}

bool MoviePlayer::playMovie(Common::File *file, uint32 offset, GamosEngine *gamos) {
    if (!init(file, offset, gamos))
        return error();

    while (true) {
        int status = 0;

        readHdr();

        switch(_hdrBytes[0]) {
            case 0:
            status = processControlChunk();
            break;

            case 1:
            status = processImageChunk();
            break;

            case 2:
            status = processPaletteChunk();
            break;

            case 3:
            status = processSoundChunk();
            break;

            case 4:
            status = proccessMidiChunk();
            break;

            default:
            break;
        }

        while(true)
        {
            if (status == 2) {
                while(true) {
                    if ( !readHdr() )
                        return error();
                    if (_hdrBytes[0] == 0) {
                        if (_hdrBytes[1] == 0 || _hdrBytes[1] == 1)
                            break;
                    } else {
                        if (!_file->seek(_hdrValue1, SEEK_CUR))
                            return error();
                    }
                }
                status = processControlChunk();
            } else if (status == 0) {
                return error();
            } else if (status == 3) {
                return deinit();
            } else {
                break;
            }
        }
    }

    return deinit();
}

bool MoviePlayer::error() {
    deinit();
    _gamos->setErrMessage("Movie playback error.");
    return false;
}

int MoviePlayer::processControlChunk() {
    printf("%x movieProcessControl %d\n", _file->pos(), _hdrBytes[1]);

    switch(_hdrBytes[1]) {
    case 0:
        if ((uint32_t)_hdrValue1 != 0x563d2d5b || (uint32_t)_hdrValue2 != 0x5d2d3d53) {
            error();
            return 0;
        }
        return 3;

    case 1:
        _loopCount = 1;
        _loopPoint = 0;

        if (_hdrBytes[2] != 0)
            _loopCount = _hdrValue1;

        if (_hdrBytes[3] != 0)
            _frameTime = _hdrValue2;
        break;

    case 2:
        if (_hdrBytes[2] != 0) {
            _packedBufferSize = _hdrValue1;
            _packedBuffer.resize(_hdrValue1);
        }
        break;

    case 3:
        if (_hdrBytes[2] != 0) {
            _bufferSize = _hdrValue1;
            _buffer.resize(_hdrValue1);
        }
        if (_hdrBytes[3] != 0) {
            _paletteBufferSize = _hdrValue2;
            _paletteBuffer.resize(_hdrValue2);
        }
        break;

    case 4:
        if (_hdrBytes[2] != 0) {
            _soundBufferSize = _hdrValue1;
            _soundBuffer.resize(_hdrValue1);
        }
        if (_hdrBytes[3] != 0) {
            _midiBufferSize = _hdrValue2;
            _midiBuffer.resize(_hdrValue2);
        }
        break;

    case 5:
        if (_hdrBytes[2] != 0) {
            _pos.x = _hdrValue1;
        }
        if (_hdrBytes[3] != 0) {
            _pos.y = _hdrValue2; /* BUG? Originally here same _pos.x */
        }
        break;

    case 6:
        if (_hdrBytes[2] != 0) {
            _frameSize.x = _hdrValue1;
        }
        if (_hdrBytes[3] != 0) {
            _frameSize.y = _hdrValue2;
        }
        break;

    }
    return 1;
}

int MoviePlayer::processImageChunk() {
    printf("%x movieProcessImageChunk %d\n", _file->pos(), _hdrValue1);
    if (!readCompressed(_bufferSize, &_buffer))
        return 0;

    bool keepAct = true;

    if (_hdrBytes[1] == 1) {
        _forceStopMidi = false;
        //waveoutrestart()
        _screen->fillRect(_screen->getBounds(), _hdrBytes[3]);
        if (_loopCount > 1)
            _loopPoint = _file->pos();
        keepAct = false;
        _doUpdateScreen = false;
    }
    else if (_hdrBytes[1] == 2) {
        _forceStopMidi = true;
        _loopCount--;
        if (_loopCount != 0)
            _file->seek(_loopPoint, 0);
    }

    if (_hdrValue1 != 0) {
        byte *pdata = _buffer.data();
        Common::Point xy;
        Common::Point wh;

        while (true) {
            byte val = *pdata;
            pdata++;
            if ( (val & 0x40) == 0 ) {
                xy.x = _pos.x + *pdata;
                pdata++;

                if ( (val & 4) != 0 ) {
                    xy.x += *pdata * 256;
                    pdata++;
                }

                xy.y = _pos.y + *pdata;
                pdata++;

                if ( (val & 8) != 0 ) {
                    xy.y += *pdata * 256;
                    pdata++;
                }

                wh.x = *pdata;
                pdata++;

                if ( (val & 0x10) != 0 ) {
                    wh.x += *pdata * 256;
                    pdata++;
                }

                wh.y = *pdata;
                pdata++;

                if ( (val & 0x20) != 0 ) {
                    wh.y += *pdata * 256;
                    pdata++;
                }
            } else {
                xy = _pos;
                wh = _frameSize;
            }

            printf("movie blit%d %d %d %d %d\n", val & 3, xy.x, xy.y, wh.x, wh.y);
            static byte *(*blitters[4])(Common::Rect, byte *, Graphics::Surface *) =
               {&blit0,
                &blit1,
                &blit2,
                &blit3};

            pdata = blitters[val & 3](Common::Rect(xy, xy + wh), pdata, _screen->surfacePtr());

            if (_doUpdateScreen) {
                _gamos->updateScreen(false, Common::Rect(xy, xy + wh));
            }

            if (val & 0x80)
                break;
        }

    }

    if (_doUpdateScreen)
        _gamos->flushDirtyRects(true);

    uint32 tstamp = 0;
    uint8 act = processMessages(keepAct, &tstamp);

    if (act == ACT2_82)
        return 2;

    if (act == ACT2_83)
        return 3;

    if (_hdrBytes[1] == 1) {
        _gamos->updateScreen(_gamos->_fadeEffectID != 0, Common::Rect(_pos, _pos + _frameSize));

        _firstFrameTime = g_system->getMillis();
        _currentFrame = 0;
        _skippedFrames = 0;
        _doUpdateScreen = true;
    } else if (_frameTime == 0) {
        _doUpdateScreen = true;
    } else {
        int32 dtime = (tstamp - _firstFrameTime) / _currentFrame;
        if (dtime > _frameTime) {
            if (_soundBufferSize) {
                _skippedFrames++;
                if (_skippedFrames != 8)
                    _doUpdateScreen = false;
            }
        }
        else if (dtime < _frameTime) {
            while (true) {
                act = processMessages(false, &tstamp);
                if (act == ACT2_82)
                    return 2;

                if (act == ACT2_83)
                    return 3;

                if ((tstamp - _firstFrameTime) / _currentFrame >= _frameTime)
                    break;

                g_system->delayMillis(1);
            }
        }

        _skippedFrames = 0;
        _doUpdateScreen = true;
    }

    _screen->update();
    _currentFrame++;

    return 1;
}

int MoviePlayer::processPaletteChunk() {
    printf("%x movieProcessPaletteChunk\n", _file->pos());
    if (!readCompressed(_paletteBufferSize, &_paletteBuffer))
        return 0;

    if (!_gamos->usePalette(_paletteBuffer.data(), 256,  _gamos->_fadeEffectID, false))
        return 0;

    return 1;
}

int MoviePlayer::processSoundChunk() {
    printf("%x movieProcessSoundChunk\n", _file->pos());
    if (!readCompressed(_soundBufferSize, &_soundBuffer))
        return 0;
    return 1;
}

int MoviePlayer::proccessMidiChunk() {
    printf("%x movieProccessMidiChunk\n", _file->pos());

    if (_midiStarted && (_forceStopMidi == false || _hdrBytes[1] != 0)) {
        _gamos->stopMidi();
        _midiStarted = false;
    }

    if (_hdrValue1 == 0)
        return 1;

    if (_midiStarted) {
        if ( !_file->seek(_hdrValue1, SEEK_CUR) )
            return 0;
        return 1;
    }

    if (!readCompressed(_midiBufferSize, &_midiBuffer)) {
        _midiStarted = false;
        return 0;
    }

    _midiStarted = _gamos->playMidi(&_midiBuffer);

    return 1;
}

bool MoviePlayer::readHdr() {
    _file->read(_hdrBytes, 4);
    _hdrValue1 = _file->readSint32LE();
    _hdrValue2 = _file->readSint32LE();
    return true;
}

bool MoviePlayer::readCompressed(int32_t count, Common::Array<byte> *buf) {
    if (_hdrValue1 == 0)
        return true;

    if (_hdrValue1 != _hdrValue2) {
        _packedBuffer.resize(_hdrValue1);
        _file->read(_packedBuffer.data(), _hdrValue1);
        buf->resize(_hdrValue2);
        Archive::decompress(&_packedBuffer, buf);
    }
    else {
        buf->resize(_hdrValue1);
        _file->read(buf->data(), _hdrValue1);
    }
    return true;
}

byte* MoviePlayer::blit0(Common::Rect rect, byte *in, Graphics::Surface *surface) {
    int16 y = rect.top;
    while (y < rect.bottom) {
        const int count = rect.width();
        byte *out = (byte *)surface->getBasePtr(rect.left, y);
        for (int i = 0; i < count; i++) {
            *out += *in;
            in++;
            out++;
        }
        y++;
    }
    return in;
}

byte* MoviePlayer::blit1(Common::Rect rect, byte *in, Graphics::Surface *surface) {
    int16 y = rect.top;
    int16 x = rect.left;
    while (y < rect.bottom) {

        byte b = *in;
        in++;
        if (b & 0x80) {
            if ((b & 0x40) == 0) {
                int count = (b & 0x3f) + 1;
                byte *out = (byte *)surface->getBasePtr(x, y);
                for (int i = 0; i < count; i++) {
                    *out += *in;
                    in++;
                    out++;
                }
                x += count;
            } else {
                if ((b & 0x3f) == 0)
                    x = rect.right;
                else {
                    if ((b & 0x3f) != 1) {
                        int count = (b & 0x3f) + 1;
                        byte val = *in;
                        in++;
                        byte *out = (byte *)surface->getBasePtr(x, y);
                        for (int i = 0; i < count; i++) {
                            *out += val;
                            out++;
                        }
                        x += count;
                    } else {
                        int count = rect.right - x;
                        byte *out = (byte *)surface->getBasePtr(x, y);
                        for (int i = 0; i < count; i++) {
                            *out += *in;
                            in++;
                            out++;
                        }
                        x = rect.right;
                    }
                }
            }
        } else {
            x += b + 1;
        }

        if (x >= rect.right) {
            y++;
            x = rect.left;
        }
    }
    return in;
}

byte* MoviePlayer::blit2(Common::Rect rect, byte *in, Graphics::Surface *surface) {
    int16 y = rect.top;
    int16 x = rect.left;
    while (y < rect.bottom) {

        byte b = *in;
        in++;
        if (b & 0x80) {
            if ((b & 0x40) == 0) {
                x += (b & 0x3f) + 1;
            } else {
                int count = (b & 0x3f) + 1;
                byte *out = (byte *)surface->getBasePtr(x, y);
                for (int i = 0; i < count; i++) {
                    *out += *in;
                    in++;
                    out++;
                }
                x += count;
            }
        } else if (b == 0) {
            x += b + 1;
        } else if (b != 1) {
            int count = b + 1;
            byte val = *in;
            in++;
            byte *out = (byte *)surface->getBasePtr(x, y);
            for (int i = 0; i < count; i++) {
                *out += val;
                out++;
            }
            x += count;
        } else {
            int count = rect.right - x;
            byte *out = (byte *)surface->getBasePtr(x, y);
            for (int i = 0; i < count; i++) {
                *out += *in;
                in++;
                out++;
            }
            x = rect.right;
        }

        if (x >= rect.right) {
            y++;
            x = rect.left;
        }
    }
    return in;
}

byte* MoviePlayer::blit3(Common::Rect rect, byte *in, Graphics::Surface *surface) {
    int16 y = rect.top;
    int16 x = rect.left;
    while (y < rect.bottom) {

        byte b = *in;
        in++;
        if (b & 0x80) {
            if ((b & 0x40) == 0) {
                x += (b & 0x3f) + 1;
            } else {
                if ((b & 0x3f) == 0)
                    x = rect.right;
                else {
                    if ((b & 0x3f) != 1) {
                        int count = (b & 0x3f) + 1;
                        byte val = *in;
                        in++;
                        byte *out = (byte *)surface->getBasePtr(x, y);
                        for (int i = 0; i < count; i++) {
                            *out += val;
                            out++;
                        }
                        x += count;
                    } else {
                        int count = rect.right - x;
                        byte *out = (byte *)surface->getBasePtr(x, y);
                        for (int i = 0; i < count; i++) {
                            *out += *in;
                            in++;
                            out++;
                        }
                        x = rect.right;
                    }
                }
            }
        } else {
            int count = b + 1;
            byte *out = (byte *)surface->getBasePtr(x, y);
            for (int i = 0; i < count; i++) {
                *out += *in;
                in++;
                out++;
            }
            x += count;
        }

        if (x >= rect.right) {
            y++;
            x = rect.left;
        }
    }
    return in;
}


uint8 MoviePlayer::processMessages(bool keepAct, uint32 *msecs) {
	if (!keepAct)
		_messageProc->_act2 = ACT_NONE;

	Common::Event e;
	while (g_system->getEventManager()->pollEvent(e)) {
		if (e.type == Common::EVENT_QUIT) {
			//_errMessage = 1;
			return ACT2_83;
		}
		_messageProc->processMessage(e);
	}

	uint8 act = _messageProc->_act2;
	_messageProc->_act2 = ACT_NONE;
	*msecs = g_system->getMillis();
	return act;
}

}
