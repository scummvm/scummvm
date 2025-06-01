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

#include "ultima/ultima.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/misc/debugger.h"
#include "ultima/ultima8/gfx/skf_player.h"
#include "ultima/ultima8/convert/u8/convert_shape_u8.h"
#include "ultima/ultima8/filesys/raw_archive.h"
#include "ultima/ultima8/gfx/shape.h"
#include "ultima/ultima8/gfx/texture.h"
#include "ultima/ultima8/audio/music_process.h"
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/audio/raw_audio_sample.h"
#include "ultima/ultima8/gfx/fonts/font.h"
#include "ultima/ultima8/gfx/fonts/font_manager.h"
#include "ultima/ultima8/gfx/fonts/rendered_text.h"
#include "common/config-manager.h"
#include "common/system.h"

namespace Ultima {
namespace Ultima8 {

enum SKFAction {
	SKF_PlayMusic = 3,
	SKF_SlowStopMusic = 4,
	SKF_PlaySFX = 5,
	SKF_StopSFX = 6,
	SKF_SetSpeed = 7,
	SKF_FadeOut = 8,
	SKF_FadeIn = 9,
	SKF_Wait = 12,
	SKF_PlaySound = 14,
	SKF_FadeWhite = 15,
	SKF_ClearSubs = 18
};

struct SKFEvent {
	unsigned int _frame;
	SKFAction _action;
	unsigned int _data;
};

// number of steps in a fade
static const int FADESTEPS = 16; // HACK: half speed


SKFPlayer::SKFPlayer(Common::SeekableReadStream *rs, int width, int height, bool introMusicHack)
	: _width(width), _height(height), _curFrame(0), _curObject(0), _curAction(0),
	  _curEvent(0), _playing(false), _timer(0), _frameRate(15), _fadeColour(0),
	  _fadeLevel(0), _buffer(nullptr), _subs(nullptr), _introMusicHack(introMusicHack),
	  _lastUpdate(0), _subtitleY(0) {
	_skf = new RawArchive(rs);
	Common::ReadStream *eventlist = _skf->get_datasource(0);
	if (!eventlist) {
		warning("No eventlist found in SKF");
		return;
	}

	parseEventList(eventlist);
	delete eventlist;

	Graphics::Screen *screen = Ultima8Engine::get_instance()->getScreen();
	_buffer = new RenderSurface(_width, _height, screen->format);
}

SKFPlayer::~SKFPlayer() {
	for (unsigned int i = 0; i < _events.size(); ++i)
		delete _events[i];

	delete _skf;
	delete _buffer;
	delete _subs;
}

void SKFPlayer::parseEventList(Common::ReadStream *eventlist) {
	uint16 frame = eventlist->readUint16LE();
	while (frame != 0xFFFF) {
		SKFEvent *ev = new SKFEvent;
		ev->_frame = frame;
		ev->_action = static_cast<SKFAction>(eventlist->readUint16LE());
		ev->_data = eventlist->readUint16LE();
		_events.push_back(ev);

		frame = eventlist->readUint16LE();
	}
}

void SKFPlayer::start() {
	uint32 color = TEX32_PACK_RGB(0, 0, 0);
	_buffer->fill32(color, 0, 0, _width, _height);
	MusicProcess *musicproc = MusicProcess::get_instance();
	if (musicproc) musicproc->playMusic(0);
	_playing = true;
	_lastUpdate = g_system->getMillis();
}

void SKFPlayer::stop() {
	MusicProcess *musicproc = MusicProcess::get_instance();
	if (musicproc && !_introMusicHack) musicproc->playMusic(0);
	_playing = false;
}

void SKFPlayer::paint(RenderSurface *surf, int /*lerp*/) {
	if (!_buffer) return;

	if (!_fadeLevel) {
		Common::Rect srcRect(_width, _height);
		surf->Blit(*_buffer->getRawSurface(), srcRect, 0, 0);
		if (_subs)
			_subs->draw(surf, 60, _subtitleY);
	} else {
		uint32 fade = TEX32_PACK_RGBA(_fadeColour, _fadeColour, _fadeColour,
		                              (_fadeLevel * 255) / FADESTEPS);
		Common::Rect srcRect(_width, _height);
		surf->FadedBlit(*_buffer->getRawSurface(), srcRect, 0, 0, fade);
		if (_subs)
			_subs->drawBlended(surf, 60, _subtitleY, fade);
	}
}

void SKFPlayer::run() {
	if (!_playing || !_buffer) return;

	MusicProcess *musicproc = MusicProcess::get_instance();

	// if doing something, continue
	if (_curAction) {
		if (_curAction == SKF_FadeOut || _curAction == SKF_FadeWhite) {
			_fadeLevel++;
			if (_fadeLevel == FADESTEPS) _curAction = 0; // done
		} else if (_curAction == SKF_FadeIn) {
			_fadeLevel--;
			if (_fadeLevel == 0) _curAction = 0; // done
		} else if (_curAction == SKF_SlowStopMusic) {
			if (!musicproc || !musicproc->isFading()) {
				if (musicproc)
					musicproc->playMusic(0); // stop playback
				_curAction = 0; // done
			} else {
				// continue to wait for fade to finish
				return;
			}
		} else {
			debugC(kDebugVideo, "Unknown fade action: %u", _curAction);
		}
	}

	// CHECKME: this timing may not be accurate enough...
	uint32 now = g_system->getMillis();
	if (_lastUpdate + (1000 / _frameRate) > now) return;

	_lastUpdate += (1000 / _frameRate);

	// if waiting, continue to wait
	if (_timer) {
		_timer--;
		return;
	}

	Font *redfont;
	redfont = FontManager::get_instance()->getGameFont(6, true);

	AudioProcess *audioproc = AudioProcess::get_instance();

	bool subtitles = ConfMan.getBool("subtitles");
	bool speechMute = ConfMan.getBool("speech_mute");

	// handle _events for the current frame
	while (_curEvent < _events.size() && _events[_curEvent]->_frame <= _curFrame) {
		debugCN(kDebugVideo, "Event %u: ", _curEvent);
		switch (_events[_curEvent]->_action) {
		case SKF_FadeOut:
			_curAction = SKF_FadeOut;
			_fadeColour = 0;
			_fadeLevel = 0;
			debugC(kDebugVideo, "FadeOut");
			break;
		case SKF_FadeIn:
			_curAction = SKF_FadeIn;
			_fadeLevel = FADESTEPS;
			debugC(kDebugVideo, "FadeIn");
			break;
		case SKF_FadeWhite:
			_curAction = SKF_FadeWhite;
			_fadeColour = 0xFF;
			_fadeLevel = 0;
			debugC(kDebugVideo, "FadeWhite");
			break;
		case SKF_Wait:
			debugC(kDebugVideo, "Wait %u", _events[_curEvent]->_data);
			_timer = _events[_curEvent]->_data;
			_curEvent++;
			return;
		case SKF_PlayMusic:
			debugC(kDebugVideo, "PlayMusic %u", _events[_curEvent]->_data);
			if (musicproc) musicproc->playMusic(_events[_curEvent]->_data);
			break;
		case SKF_SlowStopMusic:
			debugC(kDebugVideo, "SlowStopMusic");
			if (musicproc)
				musicproc->fadeMusic(1500);
			_curAction = SKF_SlowStopMusic;
			break;
		case SKF_PlaySFX:
			debugC(kDebugVideo, "PlaySFX %u", _events[_curEvent]->_data);
			if (audioproc) audioproc->playSFX(_events[_curEvent]->_data, 0x60, 0, 0);
			break;
		case SKF_StopSFX:
			debugC(kDebugVideo, "StopSFX %u", _events[_curEvent]->_data);
			if (audioproc) audioproc->stopSFX(_events[_curEvent]->_data, 0);
			break;
		case SKF_SetSpeed:
			debugC(kDebugVideo, "SetSpeed %u", _events[_curEvent]->_data);
//			_frameRate = _events[_curEvent]->_data;
			break;
		case SKF_PlaySound: {
			debugC(kDebugVideo, "PlaySound %u", _events[_curEvent]->_data);

			if (!speechMute && audioproc) {
				uint8 *buf = _skf->get_object(_events[_curEvent]->_data);
				uint32 bufsize = _skf->get_size(_events[_curEvent]->_data);
				AudioSample *s;
				uint32 rate = buf[6] + (buf[7] << 8);
				bool stereo = (buf[8] == 2);
				s = new RawAudioSample(buf + 34, bufsize - 34,
				                                  rate, true, stereo);
				audioproc->playSample(s, 0x60, 0, true);
				// FIXME: memory leak! (sample is never deleted)
			}

			// subtitles
			char *textbuf = reinterpret_cast<char *>(
			                    _skf->get_object(_events[_curEvent]->_data - 1));
			uint32 textsize = _skf->get_size(_events[_curEvent]->_data - 1);
			if (subtitles && textsize > 7) {
				Std::string subtitle = (textbuf + 6);
				delete _subs;
				_subtitleY = textbuf[4] + (textbuf[5] << 8);
				unsigned int remaining;
				_subs = redfont->renderText(subtitle, remaining, 200, 0,
				                           Font::TEXT_CENTER);
			}
			delete textbuf;


			break;
		}
		case SKF_ClearSubs:
			debugC(kDebugVideo, "ClearSubs");
			delete _subs;
			_subs = nullptr;
			break;
		default:
			debugC(kDebugVideo, "Unknown action %d", _events[_curEvent]->_action);
			break;
		}

		_curEvent++;
	}

	_curFrame++;

	uint16 objecttype = 0;
	do {
		_curObject++;
		if (_curObject >= _skf->getCount()) {
			stop(); // done
			return;
		}

		// read object
		Common::SeekableReadStream * object = _skf->get_datasource(_curObject);
		if (!object)
			continue;

		objecttype = object->size() > 2 ? object->readUint16LE() : 0;

		debugC(kDebugVideo, "Object %u/%u, type = %u", _curObject, _skf->getCount(),  objecttype);

		switch (objecttype) {
		case 1:
			_palette.load(*object);
			_palette.updateNativeMap(_buffer->getRawSurface()->format);
			break;
		case 2: {
			object->seek(0);
			Shape *shape = new Shape(object, &U8SKFShapeFormat);
			shape->setPalette(&_palette);
			_buffer->BeginPainting();
			_buffer->Paint(shape, 0, 0, 0);
			_buffer->EndPainting();
			delete shape;
			break;
		}
		default:
			break;
		}

		delete object;
	} while (objecttype != 2);

	_timer = 1; // HACK! timing is rather broken currently...
}

} // End of namespace Ultima8
} // End of namespace Ultima
