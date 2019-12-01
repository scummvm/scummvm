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

#include "ultima8/misc/pent_include.h"
#include "SKFPlayer.h"

#include "u8/ConvertShapeU8.h"
#include "raw_archive.h"
#include "Shape.h"
#include "ultima8/graphics/texture.h"
#include "SoftRenderSurface.h"
#include "ultima8/graphics/palette_manager.h"
#include "ultima8/audio/music_process.h"
#include "ultima8/audio/audio_process.h"
#include "ultima8/filesys/idata_source.h"
#include "ultima8/audio/audio_mixer.h"
#include "RawAudioSample.h"
#include "ultima8/graphics/fonts/font.h"
#include "ultima8/graphics/fonts/font_manager.h"
#include "RenderedText.h"

#include "SDL.h"

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
	unsigned int frame;
	SKFAction action;
	unsigned int data;
};

// number of steps in a fade
static const int FADESTEPS = 16; // HACK: half speed


SKFPlayer::SKFPlayer(RawArchive *movie, int width_, int height_, bool introMusicHack_)
	: width(width_), height(height_), skf(movie),
	  curframe(0), curobject(0), curaction(0), curevent(0), playing(false),
	  timer(0), framerate(15), fadecolour(0), fadelevel(0), buffer(0), subs(0),
	  introMusicHack(introMusicHack_) {
	IDataSource *eventlist = skf->get_datasource(0);
	if (!eventlist) {
		perr << "No eventlist found in SKF" << std::endl;
		return;
	}

	parseEventList(eventlist);
	delete eventlist;

	buffer = RenderSurface::CreateSecondaryRenderSurface(width, height);
}

SKFPlayer::~SKFPlayer() {
	for (unsigned int i = 0; i < events.size(); ++i)
		delete events[i];

	delete skf;
	delete buffer;
	delete subs;
}

void SKFPlayer::parseEventList(IDataSource *eventlist) {
	uint16 frame = eventlist->read2();
	while (frame != 0xFFFF) {
		SKFEvent *ev = new SKFEvent;
		ev->frame = frame;
		ev->action = static_cast<SKFAction>(eventlist->read2());
		ev->data = eventlist->read2();
		events.push_back(ev);

		frame = eventlist->read2();
	}
}

void SKFPlayer::start() {
	buffer->BeginPainting();
	buffer->Fill32(0, 0, 0, width, height);
	buffer->EndPainting();
	MusicProcess *musicproc = MusicProcess::get_instance();
	if (musicproc) musicproc->playMusic(0);
	playing = true;
	lastupdate = SDL_GetTicks();
}

void SKFPlayer::stop() {
	MusicProcess *musicproc = MusicProcess::get_instance();
	if (musicproc && !introMusicHack) musicproc->playMusic(0);
	playing = false;
}

void SKFPlayer::paint(RenderSurface *surf, int /*lerp*/) {
	if (!buffer) return;

	Texture *tex = buffer->GetSurfaceAsTexture();

	if (!fadelevel) {
		surf->Blit(tex, 0, 0, width, height, 0, 0);
		if (subs)
			subs->draw(surf, 60, subtitley);
	} else {
		uint32 fade = TEX32_PACK_RGBA(fadecolour, fadecolour, fadecolour,
		                              (fadelevel * 255) / FADESTEPS);
		surf->FadedBlit(tex, 0, 0, width, height, 0, 0, fade);
		if (subs)
			subs->drawBlended(surf, 60, subtitley, fade);
	}
}

void SKFPlayer::run() {
	if (!playing || !buffer) return;

	// if doing something, continue
	if (curaction) {
		if (curaction == SKF_FadeOut || curaction == SKF_FadeWhite) {
			fadelevel++;
			if (fadelevel == FADESTEPS) curaction = 0; // done
		} else if (curaction == SKF_FadeIn) {
			fadelevel--;
			if (fadelevel == 0) curaction = 0; // done
		} else {
			pout << "Unknown fade action: " << curaction << std::endl;
		}
	}

	// CHECKME: this timing may not be accurate enough...
	uint32 now = SDL_GetTicks();
	if (lastupdate + (1000 / framerate) > now) return;

	lastupdate += (1000 / framerate);

	// if waiting, continue to wait
	if (timer) {
		timer--;
		return;
	}

	Pentagram::Font *redfont;
	redfont = FontManager::get_instance()->getGameFont(6, true);

	MusicProcess *musicproc = MusicProcess::get_instance();
	AudioProcess *audioproc = AudioProcess::get_instance();

	// handle events for the current frame
	while (curevent < events.size() && events[curevent]->frame <= curframe) {
//		pout << "event " << curevent << std::endl;
		switch (events[curevent]->action) {
		case SKF_FadeOut:
			curaction = SKF_FadeOut;
			fadecolour = 0;
			fadelevel = 0;
//			pout << "FadeOut" << std::endl;
			break;
		case SKF_FadeIn:
			curaction = SKF_FadeIn;
			fadelevel = FADESTEPS;
//			pout << "FadeIn" << std::endl;
			break;
		case SKF_FadeWhite:
			curaction = SKF_FadeWhite;
			fadecolour = 0xFF;
			fadelevel = 0;
//			pout << "FadeWhite" << std::endl;
			break;
		case SKF_Wait:
//			pout << "Wait " << events[curevent]->data << std::endl;
			timer = events[curevent]->data;
			curevent++;
			return;
		case SKF_PlayMusic:
//			pout << "PlayMusic " << events[curevent]->data << std::endl;
			if (musicproc) musicproc->playMusic(events[curevent]->data);
			break;
		case SKF_SlowStopMusic:
			POUT("SlowStopMusic");
			if (musicproc && !introMusicHack) musicproc->playMusic(0);
			break;
		case SKF_PlaySFX:
//			pout << "PlaySFX " << events[curevent]->data << std::endl;
			if (audioproc) audioproc->playSFX(events[curevent]->data, 0x60, 0, 0);
			break;
		case SKF_StopSFX:
//			pout << "StopSFX" << events[curevent]->data << std::endl;
			if (audioproc) audioproc->stopSFX(events[curevent]->data, 0);
			break;
		case SKF_SetSpeed:
			POUT("SetSpeed " << events[curevent]->data);
//			framerate = events[curevent]->data;
			break;
		case SKF_PlaySound: {
//			pout << "PlaySound " << events[curevent]->data << std::endl;

			if (audioproc) {
				uint8 *buffer = skf->get_object(events[curevent]->data);
				uint32 bufsize = skf->get_size(events[curevent]->data);
				Pentagram::AudioSample *s;
				uint32 rate = buffer[6] + (buffer[7] << 8);
				bool stereo = (buffer[8] == 2);
				s = new Pentagram::RawAudioSample(buffer + 34, bufsize - 34,
				                                  rate, true, stereo);
				audioproc->playSample(s, 0x60, 0);
				// FIXME: memory leak! (sample is never deleted)
			}

			// subtitles
			char *textbuf = reinterpret_cast<char *>(
			                    skf->get_object(events[curevent]->data - 1));
			uint32 textsize = skf->get_size(events[curevent]->data - 1);
			if (textsize > 7) {
				std::string subtitle = (textbuf + 6);
				delete subs;
				subtitley = textbuf[4] + (textbuf[5] << 8);
				unsigned int remaining;
				subs = redfont->renderText(subtitle, remaining, 200, 0,
				                           Pentagram::Font::TEXT_CENTER);
			}
			delete textbuf;


			break;
		}
		case SKF_ClearSubs:
//			pout << "ClearSubs" << std::endl;
			delete subs;
			subs = 0;
			break;
		default:
			pout << "Unknown action" << std::endl;
			break;
		}

		curevent++;
	}

	curframe++;

	PaletteManager *palman = PaletteManager::get_instance();
	IDataSource *object;

	uint16 objecttype = 0;
	do {
		curobject++;
		if (curobject >= skf->getCount()) {
			stop(); // done
			return;
		}

		// read object
		object = skf->get_datasource(curobject);
		if (!object || object->getSize() < 2)
			continue;

		objecttype = object->read2();

//		pout << "Object " << curobject << "/" << skf->getCount()
//			 << ", type = " << objecttype << std::endl;


		if (objecttype == 1) {
			palman->load(PaletteManager::Pal_Movie, *object);
		}

		if (objecttype != 2)
			delete object;

	} while (objecttype != 2);

	if (objecttype == 2) {
		object->seek(0);
		Shape *shape = new Shape(object, &U8SKFShapeFormat);
		Pentagram::Palette *pal = palman->getPalette(PaletteManager::Pal_Movie);
		shape->setPalette(pal);
		buffer->BeginPainting();
		buffer->Paint(shape, 0, 0, 0);
		buffer->EndPainting();
		delete shape;

		delete object;
	}

	timer = 1; // HACK! timing is rather broken currently...
}

} // End of namespace Ultima8
