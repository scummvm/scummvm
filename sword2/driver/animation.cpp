/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "common/stdafx.h"
#include "common/file.h"
#include "sound/vorbis.h"
#include "sound/mp3.h"

#include "sword2/sword2.h"
#include "sword2/maketext.h"
#include "sword2/driver/animation.h"
#include "sword2/driver/d_draw.h"
#include "sword2/driver/d_sound.h"
#include "sword2/driver/menu.h"
#include "sword2/driver/render.h"

namespace Sword2 {

AnimationState::AnimationState(Sword2Engine *vm)
	: BaseAnimationState(vm->_mixer, vm->_system, 640, 480), _vm(vm) {
}

AnimationState::~AnimationState() {
}


#ifdef BACKEND_8BIT

void AnimationState::setPalette(byte *pal) {
	_vm->_graphics->setPalette(0, 256, pal, RDPAL_INSTANT);
}

#else

void AnimationState::drawTextObject(SpriteInfo *s, uint8 *src) {
	OverlayColor *dst = overlay + RENDERWIDE * (s->y) + s->x;

	// FIXME: These aren't the "right" colours, but look good to me.

	OverlayColor pen = _sys->RGBToColor(255, 255, 255);
	OverlayColor border = _sys->RGBToColor(0, 0, 0);

	for (int y = 0; y < s->h; y++) {
		for (int x = 0; x < s->w; x++) {
			switch (src[x]) {
			case 1:
				dst[x] = border;
				break;
			case 255:
				dst[x] = pen;
				break;
			default:
				break;
			}
		}
		dst += RENDERWIDE;
		src += s->w;
	}
}

void AnimationState::clearDisplay(void) {
	OverlayColor black = _sys->RGBToColor(0, 0, 0);

	for (int i = 0; i < MOVIE_WIDTH * MOVIE_HEIGHT; i++)
		overlay[i] = black;
}

void AnimationState::updateDisplay(void) {
	_sys->copy_rect_overlay(overlay, MOVIE_WIDTH, 0, 0, MOVIE_WIDTH, MOVIE_HEIGHT);
}

#endif

bool AnimationState::decodeFrame() {
#ifdef USE_MPEG2
	mpeg2_state_t state;
	const mpeg2_sequence_t *sequence_i;
	size_t size = (size_t) -1;

	do {
		state = mpeg2_parse(decoder);
		sequence_i = info->sequence;

		switch (state) {
		case STATE_BUFFER:
			size = mpgfile->read(buffer, BUFFER_SIZE);
			mpeg2_buffer(decoder, buffer, buffer + size);
			break;

		case STATE_SLICE:
		case STATE_END:
			if (info->display_fbuf) {
				/* simple audio video sync code:
				 * we calculate the actual frame by taking the elapsed audio time and try
				 * to stay inside +- 1 frame of this calculated frame number by dropping
				 * frames if we run behind and delaying if we are too fast
				 */

				/* Avoid deadlock is sound was too far ahead */
				if (bgSoundStream && !bgSound.isActive())
					return false;

				if (checkPaletteSwitch() || (bgSoundStream == NULL) ||
					((_snd->getChannelElapsedTime(bgSound) * 12) / 1000 < framenum + 1) ||
					frameskipped > 10) {
					if (frameskipped > 10) {
						warning("force frame %i redraw", framenum);
						frameskipped = 0;
					}
#ifdef BACKEND_8BIT
					_vm->_graphics->plotYUV(lut, sequence_i->width, sequence_i->height, info->display_fbuf->buf);
#else
					plotYUV(lookup, sequence_i->width, sequence_i->height, info->display_fbuf->buf);
#endif

					if (bgSoundStream) {
						while ((_snd->getChannelElapsedTime(bgSound) * 12) / 1000 < framenum)
							_sys->delay_msecs(10);
					} else {
						ticks += 83;
						_vm->sleepUntil(ticks);
					}

#ifdef BACKEND_8BIT
					_vm->_graphics->setNeedFullRedraw();
#endif

				} else {
					warning("dropped frame %i", framenum);
					frameskipped++;
				}

#ifdef BACKEND_8BIT
				buildLookup(palnum + 1, lutcalcnum);
#endif

				framenum++;
				return true;

			}
			break;

		default:
			break;
		}
	} while (size);
#endif
	return false;
}

MovieInfo MoviePlayer::_movies[] = {
	{ "carib",    222 },
	{ "escape",   187 },
	{ "eye",      248 },
	{ "finale",  1485 },
	{ "guard",     75 },
	{ "intro",   1800 },
	{ "jungle",   186 },
	{ "museum",   167 },
	{ "pablo",     75 },
	{ "pyramid",   60 },
	{ "quaram",   184 },
	{ "river",    656 },
	{ "sailing",  138 },
	{ "shaman",   788 },
	{ "stone1",    34 },
	{ "stone2",   282 },
	{ "stone3",    65 },
	{ "demo",      60 },
	{ "enddemo",  110 }
};

MoviePlayer::MoviePlayer(Sword2Engine *vm)
	: _vm(vm), _snd(_vm->_mixer), _sys(_vm->_system), _textSurface(NULL) {
}

void MoviePlayer::openTextObject(MovieTextObject *obj) {
	if (obj->textSprite)
		_vm->_graphics->createSurface(obj->textSprite, &_textSurface);
}

void MoviePlayer::closeTextObject(MovieTextObject *obj) {
	if (_textSurface) {
		_vm->_graphics->deleteSurface(_textSurface);
		_textSurface = NULL;
	}
}

void MoviePlayer::drawTextObject(AnimationState *anim, MovieTextObject *obj) {
	if (obj->textSprite && _textSurface) {
#ifdef BACKEND_8BIT
		_vm->_graphics->drawSurface(obj->textSprite, _textSurface);
#else
		if (anim)
			anim->drawTextObject(obj->textSprite, _textSurface);
		else
			_vm->_graphics->drawSurface(obj->textSprite, _textSurface);
#endif
	}
}

/**
 * Plays an animated cutscene.
 * @param filename the file name of the cutscene file
 * @param text the subtitles and voiceovers for the cutscene
 * @param musicOut lead-out music
 */

int32 MoviePlayer::play(const char *filename, MovieTextObject *text[], uint8 *musicOut) {
#ifdef USE_MPEG2
	uint frameCounter = 0, textCounter = 0;
	PlayingSoundHandle handle;
	bool skipCutscene = false, textVisible = false;
	uint32 flags = SoundMixer::FLAG_16BITS;
	bool startNextText = false;

	uint8 oldPal[1024];
	memcpy(oldPal, _vm->_graphics->_palCopy, 1024);

	AnimationState *anim = new AnimationState(_vm);

	if (!anim->init(filename)) {
		delete anim;
		// Missing Files? Use the old 'Narration Only' hack
		playDummy(filename, text, musicOut);
		return RD_OK;
	}

	_vm->_graphics->clearScene();

#ifndef SCUMM_BIG_ENDIAN
	flags |= SoundMixer::FLAG_LITTLE_ENDIAN;
#endif

	int i;
	uint leadOutFrame = (uint) -1;

	for (i = 0; i < ARRAYSIZE(_movies); i++) {
		if (scumm_stricmp(filename, _movies[i].name) == 0) {
			if (_movies[i].frames >= 60)
				leadOutFrame = _movies[i].frames - 60;
			else
				leadOutFrame = 0;
			break;
		}
	}

	if (i == ARRAYSIZE(_movies))
		warning("Unknown movie, '%s'", filename);

	while (anim->decodeFrame()) {
		if (text && text[textCounter]) {
			if (frameCounter == text[textCounter]->startFrame) {
				openTextObject(text[textCounter]);
				textVisible = true;

				if (text[textCounter]->speech) {
					startNextText = true;
				}
			}

			if (startNextText && !handle.isActive()) {
				_snd->playRaw(&handle, text[textCounter]->speech, text[textCounter]->speechBufferSize, 22050, flags);
				startNextText = false;
			}

			if (frameCounter == text[textCounter]->endFrame) {
				closeTextObject(text[textCounter]);
				textCounter++;
				textVisible = false;
			}
			if (textVisible)
				drawTextObject(anim, text[textCounter]);
		}

		frameCounter++;

		if (frameCounter == leadOutFrame && musicOut)
			_vm->_sound->playFx(0, musicOut, 0, 0, RDSE_FXLEADOUT);

#ifdef BACKEND_8BIT
		_vm->_graphics->updateDisplay(true);
#else
		anim->updateDisplay();
		_vm->_graphics->updateDisplay(false);
#endif

		KeyboardEvent ke;

		if ((_vm->_input->readKey(&ke) == RD_OK && ke.keycode == 27) || _vm->_quit) {
			_snd->stopHandle(handle);
			skipCutscene = true;
			break;
		}

	}

	if (!skipCutscene) {
		// Sleep for one frame so that the last frame is displayed.
		_sys->delay_msecs(1000 / 12);
	}

#ifndef BACKEND_8BIT
	// Most movies fade to black on their own, but not all of them. Since
	// we may be hanging around in the cutscene player for a while longer,
	// waiting for the lead-out sound to finish, paint the overlay black.

	anim->clearDisplay();
#else
	_vm->_graphics->clearScene();
	_vm->_graphics->setNeedFullRedraw();
#endif

	// If the speech is still playing, redraw the subtitles. At least in
	// the English version this is most noticeable in the "carib" cutscene.

	if (textVisible && handle.isActive())
		drawTextObject(anim, text[textCounter]);

	if (text)
		closeTextObject(text[textCounter]);

#ifndef BACKEND_8BIT
	anim->updateDisplay();
#else
	_vm->_graphics->updateDisplay(true);
#endif

	// Wait for the voice to stop playing. This is to make sure
	// that we don't cut off the speech in mid-sentence, and - even
	// more importantly - that we don't free the sound buffer while
	// it's in use.

	while (handle.isActive()) {
		_vm->_graphics->updateDisplay(false);
		_sys->delay_msecs(100);
	}

	// Clear the screen again

#ifndef BACKEND_8BIT
	anim->clearDisplay();
	anim->updateDisplay();
#endif

	_vm->_graphics->clearScene();
	_vm->_graphics->setNeedFullRedraw();

	_vm->_graphics->setPalette(0, 256, oldPal, RDPAL_INSTANT);

	delete anim;

	// Wait for the lead-out to stop, if there is any.
	_vm->_sound->waitForLeadOut();

	// Lead-in and lead-out music are, as far as I can tell, only used for
	// the animated cut-scenes, so this seems like a good place to close
	// both of them.

	_vm->_sound->closeFx(-1);
	_vm->_sound->closeFx(-2);

	return RD_OK;
#else
	// No MPEG2? Use the old 'Narration Only' hack
	playDummy(filename, text, musicOut);
	return RD_OK;
#endif
}

/**
 * This just plays the cutscene with voiceovers / subtitles, in case the files
 * are missing.
 */

int32 MoviePlayer::playDummy(const char *filename, MovieTextObject *text[], uint8 *musicOut) {
	int frameCounter = 0, textCounter = 0;
	if (text) {
		uint8 oldPal[1024];
		uint8 tmpPal[1024];

		_vm->_graphics->clearScene();

		// HACK: Draw instructions
		//
		// I'm using the the menu area, because that's unlikely to be
		// touched by anything else during the cutscene.

		memset(_vm->_graphics->_buffer, 0, _vm->_graphics->_screenWide * MENUDEEP);

		uint8 msg[] = "Cutscene - Narration Only: Press ESC to exit, or visit www.scummvm.org to download cutscene videos";
		Memory *data = _vm->_fontRenderer->makeTextSprite(msg, RENDERWIDE, 255, _vm->_speechFontId);
		FrameHeader *frame = (FrameHeader *) data->ad;
		SpriteInfo msgSprite;
		uint8 *msgSurface;

		msgSprite.x = _vm->_graphics->_screenWide / 2 - frame->width / 2;
		msgSprite.y = RDMENU_MENUDEEP / 2 - frame->height / 2;
		msgSprite.w = frame->width;
		msgSprite.h = frame->height;
		msgSprite.type = RDSPR_NOCOMPRESSION;
		msgSprite.data = data->ad + sizeof(FrameHeader);

		_vm->_graphics->createSurface(&msgSprite, &msgSurface);
		_vm->_graphics->drawSurface(&msgSprite, msgSurface);
		_vm->_graphics->deleteSurface(msgSurface);
		_vm->_memory->freeMemory(data);

		// In case the cutscene has a long lead-in, start just before
		// the first line of text.

		frameCounter = text[0]->startFrame - 12;

		// Fake a palette that will hopefully make the text visible.
		// In the opening cutscene it seems to use colours 1 (black?)
		// and 255 (white?).

		memcpy(oldPal, _vm->_graphics->_palCopy, 1024);
		memset(tmpPal, 0, 1024);
		tmpPal[255 * 4 + 0] = 255;
		tmpPal[255 * 4 + 1] = 255;
		tmpPal[255 * 4 + 2] = 255;
		_vm->_graphics->setPalette(0, 256, tmpPal, RDPAL_INSTANT);

		PlayingSoundHandle handle;

		bool skipCutscene = false;

		uint32 flags = SoundMixer::FLAG_16BITS;

#ifndef SCUMM_BIG_ENDIAN
		flags |= SoundMixer::FLAG_LITTLE_ENDIAN;
#endif

		while (1) {
			if (!text[textCounter])
				break;

			if (frameCounter == text[textCounter]->startFrame) {
				_vm->_graphics->clearScene();
				openTextObject(text[textCounter]);
				drawTextObject(NULL, text[textCounter]);
				if (text[textCounter]->speech) {
					_snd->playRaw(&handle, text[textCounter]->speech, text[textCounter]->speechBufferSize, 22050, flags);
				}
			}
			if (frameCounter == text[textCounter]->endFrame) {
				closeTextObject(text[textCounter]);
				_vm->_graphics->clearScene();
				_vm->_graphics->setNeedFullRedraw();
				textCounter++;
			}

			frameCounter++;

			_vm->_graphics->updateDisplay();

			KeyboardEvent ke;

			if ((_vm->_input->readKey(&ke) == RD_OK && ke.keycode == 27) || _vm->_quit) {
				_snd->stopHandle(handle);
				skipCutscene = true;
				break;
			}

			// Simulate ~12 frames per second. I don't know what
			// frame rate the original movies had, or even if it
			// was constant, but this seems to work reasonably.

			_sys->delay_msecs(90);
		}

		// Wait for the voice to stop playing. This is to make sure
		// that we don't cut off the speech in mid-sentence, and - even
		// more importantly - that we don't free the sound buffer while
		// it's in use.

		while (handle.isActive()) {
			_vm->_graphics->updateDisplay(false);
			_sys->delay_msecs(100);
		}

		closeTextObject(text[textCounter]);

		_vm->_graphics->clearScene();
		_vm->_graphics->setNeedFullRedraw();

		// HACK: Remove the instructions created above
		Common::Rect r;

		memset(_vm->_graphics->_buffer, 0, _vm->_graphics->_screenWide * MENUDEEP);
		r.left = r.top = 0;
		r.right = _vm->_graphics->_screenWide;
		r.bottom = MENUDEEP;
		_vm->_graphics->updateRect(&r);

		// FIXME: For now, only play the lead-out music for cutscenes
		// that have subtitles.

		if (!skipCutscene && musicOut) {
			_vm->_sound->playFx(0, musicOut, 0, 0, RDSE_FXLEADOUT);
			_vm->_sound->waitForLeadOut();
		}

		_vm->_graphics->setPalette(0, 256, oldPal, RDPAL_INSTANT);
	}

	// Lead-in and lead-out music are, as far as I can tell, only used for
	// the animated cut-scenes, so this seems like a good place to close
	// both of them.

	_vm->_sound->closeFx(-1);
	_vm->_sound->closeFx(-2);

	return RD_OK;
}

} // End of namespace Sword2
