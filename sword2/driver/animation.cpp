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

void AnimationState::drawTextObject(SpriteInfo *s, byte *src) {
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

#endif

void AnimationState::clearScreen(void) {
#ifdef BACKEND_8BIT
	memset(_vm->_graphics->getScreen(), 0, MOVIE_WIDTH * MOVIE_HEIGHT);
#else
	OverlayColor black = _sys->RGBToColor(0, 0, 0);

	for (int i = 0; i < MOVIE_WIDTH * MOVIE_HEIGHT; i++)
		overlay[i] = black;
#endif
}

void AnimationState::updateScreen(void) {
#ifdef BACKEND_8BIT
	byte *buf = _vm->_graphics->getScreen() + ((480 - MOVIE_HEIGHT) / 2) * RENDERWIDE + (640 - MOVIE_WIDTH) / 2;

	_vm->_system->copyRectToScreen(buf, MOVIE_WIDTH, (640 - MOVIE_WIDTH) / 2, (480 - MOVIE_HEIGHT) / 2, MOVIE_WIDTH, MOVIE_HEIGHT);
#else
	_sys->copyRectToOverlay(overlay, MOVIE_WIDTH, 0, 0, MOVIE_WIDTH, MOVIE_HEIGHT);
#endif
	_vm->_system->updateScreen();
}

void AnimationState::drawYUV(int width, int height, byte *const *dat) {
#ifdef BACKEND_8BIT
	_vm->_graphics->plotYUV(lut, width, height, dat);
#else
	plotYUV(width, height, dat);
#endif
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

int32 MoviePlayer::play(const char *filename, MovieTextObject *text[], byte *musicOut) {
	// This happens if the user quits during the "eye" smacker
	if (_vm->_quit)
		return RD_OK;

#ifdef USE_MPEG2
	uint frameCounter = 0, textCounter = 0;
	PlayingSoundHandle handle;
	bool skipCutscene = false, textVisible = false;
	uint32 flags = SoundMixer::FLAG_16BITS;
	bool startNextText = false;

	byte oldPal[256 * 4];
	memcpy(oldPal, _vm->_graphics->_palette, sizeof(oldPal));

	AnimationState *anim = new AnimationState(_vm);

	if (!anim->init(filename)) {
		delete anim;
		// Missing Files? Use the old 'Narration Only' hack
		playDummy(filename, text, musicOut);
		return RD_OK;
	}

#ifndef BACKEND_8BIT
	// Clear the screen, because whatever is on it will be visible when the
	// overlay is removed.
	_vm->_graphics->clearScene();
	_vm->_graphics->updateDisplay();
#endif

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

	while (!skipCutscene && anim->decodeFrame()) {
		// The frame has been drawn. Now draw the subtitles, if any,
		// before updating the screen.

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

		anim->updateScreen();

		frameCounter++;

		if (frameCounter == leadOutFrame && musicOut)
			_vm->_sound->playFx(0, musicOut, 0, 0, RDSE_FXLEADOUT);

		OSystem::Event event;
		while (_sys->pollEvent(event)) {
			switch (event.type) {
#ifndef BACKEND_8BIT
			case OSystem::EVENT_SCREEN_CHANGED:
				anim->buildLookup();
				break;
#endif
			case OSystem::EVENT_KEYDOWN:
				if (event.kbd.keycode == 27)
					skipCutscene = true;
				break;
			case OSystem::EVENT_QUIT:
				_vm->closeGame();
				skipCutscene = true;
				break;
			default:
				break;
			}
		}
	}

	if (!skipCutscene) {
		// Sleep for one frame so that the last frame is displayed.
		_sys->delayMillis(1000 / 12);
	}

	// Most movies fade to black on their own, but not all of them. Since
	// we may be hanging around in the cutscene player for a while longer,
	// waiting for the lead-out sound to finish, paint the overlay black.

	anim->clearScreen();

	// If the speech is still playing, redraw the subtitles. At least in
	// the English version this is most noticeable in the "carib" cutscene.

	if (textVisible && handle.isActive())
		drawTextObject(anim, text[textCounter]);

	if (text)
		closeTextObject(text[textCounter]);

	anim->updateScreen();

	// Wait for the voice to stop playing. This is to make sure
	// that we don't cut off the speech in mid-sentence, and - even
	// more importantly - that we don't free the sound buffer while
	// it's in use.

	if (skipCutscene)
		_snd->stopHandle(handle);

	while (handle.isActive()) {
		_vm->_graphics->updateDisplay(false);
		_sys->delayMillis(100);
	}

	// Clear the screen again

	anim->clearScreen();
	anim->updateScreen();

	_vm->_graphics->setPalette(0, 256, oldPal, RDPAL_INSTANT);

	delete anim;

	// Wait for the lead-out to stop, if there is any.
	_vm->_sound->waitForLeadOut();

	// Lead-in and lead-out music are, as far as I can tell, only used for
	// the animated cut-scenes, so this seems like a good place to close
	// both of them.

	_vm->_sound->stopFx(-1);
	_vm->_sound->stopFx(-2);

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

int32 MoviePlayer::playDummy(const char *filename, MovieTextObject *text[], byte *musicOut) {
	int frameCounter = 0, textCounter = 0;
	if (text) {
		byte oldPal[256 * 4];
		byte tmpPal[256 * 4];

		_vm->_graphics->clearScene();

		// HACK: Draw instructions
		//
		// I'm using the the menu area, because that's unlikely to be
		// touched by anything else during the cutscene.

		memset(_vm->_graphics->_buffer, 0, _vm->_graphics->_screenWide * MENUDEEP);

#ifdef USE_MPEG2
		byte msg[] = "Cutscene - Narration Only: Press ESC to exit, or visit www.scummvm.org to download cutscene videos";
#else
		byte msg[] = "Cutscene - Narration Only: Press ESC to exit, or recompile ScummVM with MPEG2 support";
#endif
		byte *data = _vm->_fontRenderer->makeTextSprite(msg, RENDERWIDE, 255, _vm->_speechFontId);
		FrameHeader *frame = (FrameHeader *) data;
		SpriteInfo msgSprite;
		byte *msgSurface;

		msgSprite.x = _vm->_graphics->_screenWide / 2 - frame->width / 2;
		msgSprite.y = RDMENU_MENUDEEP / 2 - frame->height / 2;
		msgSprite.w = frame->width;
		msgSprite.h = frame->height;
		msgSprite.type = RDSPR_NOCOMPRESSION;
		msgSprite.data = data + sizeof(FrameHeader);

		_vm->_graphics->createSurface(&msgSprite, &msgSurface);
		_vm->_graphics->drawSurface(&msgSprite, msgSurface);
		_vm->_graphics->deleteSurface(msgSurface);
		free(data);

		// In case the cutscene has a long lead-in, start just before
		// the first line of text.

		frameCounter = text[0]->startFrame - 12;

		// Fake a palette that will hopefully make the text visible.
		// In the opening cutscene it seems to use colours 1 (black?)
		// and 255 (white?).

		memcpy(oldPal, _vm->_graphics->_palette, sizeof(oldPal));
		memset(tmpPal, 0, sizeof(tmpPal));
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

			KeyboardEvent *ke = _vm->keyboardEvent();

			if ((ke && ke->keycode == 27) || _vm->_quit) {
				_snd->stopHandle(handle);
				skipCutscene = true;
				break;
			}

			// Simulate ~12 frames per second. I don't know what
			// frame rate the original movies had, or even if it
			// was constant, but this seems to work reasonably.

			_sys->delayMillis(90);
		}

		// Wait for the voice to stop playing. This is to make sure
		// that we don't cut off the speech in mid-sentence, and - even
		// more importantly - that we don't free the sound buffer while
		// it's in use.

		while (handle.isActive()) {
			_vm->_graphics->updateDisplay(false);
			_sys->delayMillis(100);
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

	_vm->_sound->stopFx(-1);
	_vm->_sound->stopFx(-2);

	return RD_OK;
}

} // End of namespace Sword2
