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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/graphics/skf_player.h"
#include "ultima/ultima8/convert/u8/convert_shape_u8.h"
#include "ultima/ultima8/filesys/raw_archive.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/graphics/texture.h"
#include "ultima/ultima8/graphics/soft_render_surface.h"
#include "ultima/ultima8/graphics/palette_manager.h"
#include "ultima/ultima8/audio/music_process.h"
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/audio/audio_mixer.h"
#include "ultima/ultima8/audio/raw_audio_sample.h"
#include "ultima/ultima8/graphics/fonts/font.h"
#include "ultima/ultima8/graphics/fonts/font_manager.h"
#include "ultima/ultima8/graphics/fonts/rendered_text.h"
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
		perr << "No eventlist found in SKF" << Std::endl;
		return;
	}

	parseEventList(eventlist);
	delete eventlist;

	// TODO: Slight hack.. clean me up.
	if (RenderSurface::getPixelFormat().bpp() == 16)
		_buffer = new SoftRenderSurface<uint16>(new Graphics::ManagedSurface(_width, _height, RenderSurface::getPixelFormat()));
	else
		_buffer = new SoftRenderSurface<uint32>(new Graphics::ManagedSurface(_width, _height, RenderSurface::getPixelFormat()));
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
	_buffer->Fill32(0, 0, 0, _width, _height);
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
		surf->Blit(_buffer->getRawSurface(), 0, 0, _width, _height, 0, 0);
		if (_subs)
			_subs->draw(surf, 60, _subtitleY);
	} else {
		uint32 fade = TEX32_PACK_RGBA(_fadeColour, _fadeColour, _fadeColour,
		                              (_fadeLevel * 255) / FADESTEPS);
		surf->FadedBlit(_buffer->getRawSurface(), 0, 0, _width, _height, 0, 0, fade);
		if (_subs)
			_subs->drawBlended(surf, 60, _subtitleY, fade);
	}
}

void SKFPlayer::run() {
	if (!_playing || !_buffer) return;

	// if doing something, continue
	if (_curAction) {
		if (_curAction == SKF_FadeOut || _curAction == SKF_FadeWhite) {
			_fadeLevel++;
			if (_fadeLevel == FADESTEPS) _curAction = 0; // done
		} else if (_curAction == SKF_FadeIn) {
			_fadeLevel--;
			if (_fadeLevel == 0) _curAction = 0; // done
		} else {
			pout << "Unknown fade action: " << _curAction << Std::endl;
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

	MusicProcess *musicproc = MusicProcess::get_instance();
	AudioProcess *audioproc = AudioProcess::get_instance();

	// handle _events for the current frame
	while (_curEvent < _events.size() && _events[_curEvent]->_frame <= _curFrame) {
//		pout << "event " << _curEvent << Std::endl;
		switch (_events[_curEvent]->_action) {
		case SKF_FadeOut:
			_curAction = SKF_FadeOut;
			_fadeColour = 0;
			_fadeLevel = 0;
//			pout << "FadeOut" << Std::endl;
			break;
		case SKF_FadeIn:
			_curAction = SKF_FadeIn;
			_fadeLevel = FADESTEPS;
//			pout << "FadeIn" << Std::endl;
			break;
		case SKF_FadeWhite:
			_curAction = SKF_FadeWhite;
			_fadeColour = 0xFF;
			_fadeLevel = 0;
//			pout << "FadeWhite" << Std::endl;
			break;
		case SKF_Wait:
//			pout << "Wait " << _events[_curEvent]->_data << Std::endl;
			_timer = _events[_curEvent]->_data;
			_curEvent++;
			return;
		case SKF_PlayMusic:
//			pout << "PlayMusic " << _events[_curEvent]->_data << Std::endl;
			if (musicproc) musicproc->playMusic(_events[_curEvent]->_data);
			break;
		case SKF_SlowStopMusic:
//			pout << "SlowStopMusic" << Std::endl;
			if (musicproc && !_introMusicHack) musicproc->playMusic(0);
			break;
		case SKF_PlaySFX:
//			pout << "PlaySFX " << _events[_curEvent]->_data << Std::endl;
			if (audioproc) audioproc->playSFX(_events[_curEvent]->_data, 0x60, 0, 0);
			break;
		case SKF_StopSFX:
//			pout << "StopSFX" << _events[_curEvent]->_data << Std::endl;
			if (audioproc) audioproc->stopSFX(_events[_curEvent]->_data, 0);
			break;
		case SKF_SetSpeed:
//			pout << "SetSpeed " << _events[_curEvent]->_data << Std::endl;
//			_frameRate = _events[_curEvent]->_data;
			break;
		case SKF_PlaySound: {
//			pout << "PlaySound " << _events[_curEvent]->_data << Std::endl;

			if (audioproc) {
				uint8 *buf = _skf->get_object(_events[_curEvent]->_data);
				uint32 bufsize = _skf->get_size(_events[_curEvent]->_data);
				AudioSample *s;
				uint32 rate = buf[6] + (buf[7] << 8);
				bool stereo = (buf[8] == 2);
				s = new RawAudioSample(buf + 34, bufsize - 34,
				                                  rate, true, stereo);
				audioproc->playSample(s, 0x60, 0);
				// FIXME: memory leak! (sample is never deleted)
			}

			// subtitles
			char *textbuf = reinterpret_cast<char *>(
			                    _skf->get_object(_events[_curEvent]->_data - 1));
			uint32 textsize = _skf->get_size(_events[_curEvent]->_data - 1);
			if (textsize > 7) {
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
//			pout << "ClearSubs" << Std::endl;
			delete _subs;
			_subs = nullptr;
			break;
		default:
			pout << "Unknown action" << Std::endl;
			break;
		}

		_curEvent++;
	}

	_curFrame++;

	PaletteManager *palman = PaletteManager::get_instance();
	IDataSource *object;

	uint16 objecttype = 0;
	do {
		_curObject++;
		if (_curObject >= _skf->getCount()) {
			stop(); // done
			return;
		}

		// read object
		object = _skf->get_datasource(_curObject);
		if (!object || object->size() < 2)
			continue;

		objecttype = object->readUint16LE();

//		pout << "Object " << _curObject << "/" << _skf->getCount()
//			 << ", type = " << objecttype << Std::endl;


		if (objecttype == 1) {
			palman->load(PaletteManager::Pal_Movie, *object);
		}

		if (objecttype != 2)
			delete object;

	} while (objecttype != 2);

	if (objecttype == 2) {
		object->seek(0);
		Shape *shape = new Shape(object, &U8SKFShapeFormat);
		Palette *pal = palman->getPalette(PaletteManager::Pal_Movie);
		shape->setPalette(pal);
		_buffer->BeginPainting();
		_buffer->Paint(shape, 0, 0, 0);
		_buffer->EndPainting();
		delete shape;

		delete object;
	}

	_timer = 1; // HACK! timing is rather broken currently...
}

} // End of namespace Ultima8
} // End of namespace Ultima
