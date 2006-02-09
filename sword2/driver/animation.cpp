/* Copyright (C) 1994-1998 Revolution Software Ltd.
 * Copyright (C) 2003-2006 The ScummVM project
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
 * $URL$
 * $Id$
 */

#include "common/stdafx.h"
#include "common/file.h"
#include "common/config-manager.h"
#include "common/system.h"
#include "sound/vorbis.h"
#include "sound/mp3.h"

#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/maketext.h"
#include "sword2/resman.h"
#include "sword2/sound.h"
#include "sword2/driver/animation.h"

namespace Sword2 {

AnimationState::AnimationState(Sword2Engine *vm)
	: BaseAnimationState(vm->_mixer, vm->_system, 640, 480), _vm(vm) {
}

AnimationState::~AnimationState() {
}

#ifdef BACKEND_8BIT

void AnimationState::setPalette(byte *pal) {
	_vm->_screen->setPalette(0, 256, pal, RDPAL_INSTANT);
}

#else

void AnimationState::drawTextObject(SpriteInfo *s, byte *src) {
	OverlayColor *dst = _overlay + RENDERWIDE * s->y + s->x;

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

void AnimationState::clearScreen() {
#ifdef BACKEND_8BIT
	memset(_vm->_screen->getScreen(), 0, _movieWidth * _movieHeight);
#else
	OverlayColor black = _sys->RGBToColor(0, 0, 0);

	for (int i = 0; i < _movieWidth * _movieHeight; i++)
		_overlay[i] = black;
#endif
}

void AnimationState::updateScreen() {
#ifdef BACKEND_8BIT
	byte *buf = _vm->_screen->getScreen() + ((480 - _movieHeight) / 2) * RENDERWIDE + (640 - _movieWidth) / 2;

	_vm->_system->copyRectToScreen(buf, _movieWidth, (640 - _movieWidth) / 2, (480 - _movieHeight) / 2, _movieWidth, _movieHeight);
#else
	_sys->copyRectToOverlay(_overlay, _movieWidth, 0, 0, _movieWidth, _movieHeight);
#endif
	_vm->_system->updateScreen();
}

void AnimationState::drawYUV(int width, int height, byte *const *dat) {
#ifdef BACKEND_8BIT
	_vm->_screen->plotYUV(_lut, width, height, dat);
#else
	plotYUV(width, height, dat);
#endif
}

MovieInfo MoviePlayer::_movies[] = {
	{ "carib",    222, false },
	{ "escape",   187, false },
	{ "eye",      248, false },
	{ "finale",  1485, false },
	{ "guard",     75, false },
	{ "intro",   1800, false },
	{ "jungle",   186, false },
	{ "museum",   167, false },
	{ "pablo",     75, false },
	{ "pyramid",   60, false },
	{ "quaram",   184, false },
	{ "river",    656, false },
	{ "sailing",  138, false },
	{ "shaman",   788, true  },
	{ "stone1",    34, false },
	{ "stone2",   282, false },
	{ "stone3",    65, false },
	{ "demo",      60, false },
	{ "enddemo",  110, false }
};

MoviePlayer::MoviePlayer(Sword2Engine *vm)
	: _vm(vm), _snd(_vm->_mixer), _sys(_vm->_system), _textSurface(NULL) {
}

void MoviePlayer::openTextObject(MovieTextObject *obj) {
	if (obj->textSprite)
		_vm->_screen->createSurface(obj->textSprite, &_textSurface);
}

void MoviePlayer::closeTextObject(MovieTextObject *obj) {
	if (_textSurface) {
		_vm->_screen->deleteSurface(_textSurface);
		_textSurface = NULL;
	}
}

void MoviePlayer::drawTextObject(AnimationState *anim, MovieTextObject *obj) {
	if (obj->textSprite && _textSurface) {
#ifdef BACKEND_8BIT
		_vm->_screen->drawSurface(obj->textSprite, _textSurface);
#else
		if (anim)
			anim->drawTextObject(obj->textSprite, _textSurface);
		else
			_vm->_screen->drawSurface(obj->textSprite, _textSurface);
#endif
	}
}

/**
 * Plays an animated cutscene.
 * @param filename the file name of the cutscene file
 * @param text the subtitles and voiceovers for the cutscene
 * @param leadInRes lead-in music resource id
 * @param leadOutRes lead-out music resource id
 */

int32 MoviePlayer::play(const char *filename, MovieTextObject *text[], int32 leadInRes, int32 leadOutRes) {
	Audio::SoundHandle leadInHandle;

	// This happens if the user quits during the "eye" smacker
	if (_vm->_quit)
		return RD_OK;

	if (leadInRes) {
		byte *leadIn = _vm->_resman->openResource(leadInRes);
		uint32 leadInLen = _vm->_resman->fetchLen(leadInRes) - ResHeader::size();

		assert(_vm->_resman->fetchType(leadIn) == WAV_FILE);

		leadIn += ResHeader::size();

		_vm->_sound->playFx(&leadInHandle, leadIn, leadInLen, Audio::Mixer::kMaxChannelVolume, 0, false, Audio::Mixer::kMusicSoundType);
	}

	byte *leadOut = NULL;
	uint32 leadOutLen = 0;

	if (leadOutRes) {
		leadOut = _vm->_resman->openResource(leadOutRes);
		leadOutLen = _vm->_resman->fetchLen(leadOutRes) - ResHeader::size();

		assert(_vm->_resman->fetchType(leadOut) == WAV_FILE);

		leadOut += ResHeader::size();
	}

	_leadOutFrame = (uint)-1;

	int i;

	for (i = 0; i < ARRAYSIZE(_movies); i++) {
		if (scumm_stricmp(filename, _movies[i].name) == 0) {
			_seamless = _movies[i].seamless;
			if (_movies[i].frames > 60)
				_leadOutFrame = _movies[i].frames - 60;
			break;
		}
	}

	if (i == ARRAYSIZE(_movies))
		warning("Unknown movie, '%s'", filename);

#ifdef USE_MPEG2
	playMPEG(filename, text, leadOut, leadOutLen);
#else
	// No MPEG2? Use the old 'Narration Only' hack
	playDummy(filename, text, leadOut, leadOutLen);
#endif

	_snd->stopHandle(leadInHandle);

	// Wait for the lead-out to stop, if there is any. Though don't do it
	// for seamless movies, since they are meant to blend into the rest of
	// the game.

	if (!_seamless) {
		while (_vm->_mixer->isSoundHandleActive(_leadOutHandle)) {
			_vm->_screen->updateDisplay();
			_vm->_system->delayMillis(30);
		}
	}

	if (leadInRes)
		_vm->_resman->closeResource(leadInRes);

	if (leadOutRes)
		_vm->_resman->closeResource(leadOutRes);

	return RD_OK;
}

void MoviePlayer::playMPEG(const char *filename, MovieTextObject *text[], byte *leadOut, uint32 leadOutLen) {
	uint frameCounter = 0, textCounter = 0;
	Audio::SoundHandle handle;
	bool skipCutscene = false, textVisible = false;
	uint32 flags = Audio::Mixer::FLAG_16BITS;
	bool startNextText = false;

	byte oldPal[256 * 4];
	memcpy(oldPal, _vm->_screen->getPalette(), sizeof(oldPal));

	AnimationState *anim = new AnimationState(_vm);

	if (!anim->init(filename)) {
		delete anim;
		// Missing Files? Use the old 'Narration Only' hack
		playDummy(filename, text, leadOut, leadOutLen);
		return;
	}

	// Clear the screen, because whatever is on it will be visible when the
	// overlay is removed. And if there isn't an overlay, we don't want it
	// to be visible during the cutscene. (Not all cutscenes cover the
	// entire screen.)
	_vm->_screen->clearScene();
	_vm->_screen->updateDisplay();

#ifndef SCUMM_BIG_ENDIAN
	flags |= Audio::Mixer::FLAG_LITTLE_ENDIAN;
#endif

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

			if (startNextText && !_snd->isSoundHandleActive(handle)) {
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

		if (frameCounter == _leadOutFrame && leadOut)
			_vm->_sound->playFx(&_leadOutHandle, leadOut, leadOutLen, Audio::Mixer::kMaxChannelVolume, 0, false, Audio::Mixer::kMusicSoundType);

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

	if (!_seamless) {
		// Most movies fade to black on their own, but not all of them.
		// Since we may be hanging around in the cutscene player for a
		// while longer, waiting for the lead-out sound to finish,
		// paint the overlay black.

		anim->clearScreen();
	}

	// If the speech is still playing, redraw the subtitles. At least in
	// the English version this is most noticeable in the "carib" cutscene.

	if (textVisible && _snd->isSoundHandleActive(handle))
		drawTextObject(anim, text[textCounter]);

	if (text)
		closeTextObject(text[textCounter]);

	anim->updateScreen();

	// Wait for the voice to stop playing. This is to make sure that we
	// don't cut off the speech in mid-sentence, and - even more
	// importantly - that we don't free the sound buffer while it's in use.

	if (skipCutscene)
		_snd->stopHandle(handle);

	while (_snd->isSoundHandleActive(handle)) {
		_vm->_screen->updateDisplay(false);
		_sys->delayMillis(100);
	}

	if (!_seamless) {
		// Clear the screen again
		anim->clearScreen();
		anim->updateScreen();
	}

	_vm->_screen->setPalette(0, 256, oldPal, RDPAL_INSTANT);

	delete anim;
}

/**
 * This just plays the cutscene with voiceovers / subtitles, in case the files
 * are missing.
 */

void MoviePlayer::playDummy(const char *filename, MovieTextObject *text[], byte *leadOut, uint32 leadOutLen) {
	if (!text)
		return;

	int frameCounter = 0, textCounter = 0;

	byte oldPal[256 * 4];
	byte tmpPal[256 * 4];

	_vm->_screen->clearScene();

	// HACK: Draw instructions
	//
	// I'm using the the menu area, because that's unlikely to be touched
	// by anything else during the cutscene.

	memset(_vm->_screen->getScreen(), 0, _vm->_screen->getScreenWide() * MENUDEEP);

	byte *data;

	// Russian version substituted latin characters with cyrillic. That's
	// why it renders completely unreadable with default message
	if (Common::parseLanguage(ConfMan.get("language")) == Common::RU_RUS) {
		byte msg[] = "Po\344uk - to\344\345ko pev\345: hagmute k\344abuwy Ucke\343n, u\344u nocetute ca\343t npoekta u ckava\343te budeo po\344uku";
		data = _vm->_fontRenderer->makeTextSprite(msg, RENDERWIDE, 255, _vm->_speechFontId);
	} else {
		// TODO: Translate message to other languages?
#ifdef USE_MPEG2
		byte msg[] = "Cutscene - Narration Only: Press ESC to exit, or visit www.scummvm.org to download cutscene videos";
#else
		byte msg[] = "Cutscene - Narration Only: Press ESC to exit, or recompile ScummVM with MPEG2 support";
#endif

		data = _vm->_fontRenderer->makeTextSprite(msg, RENDERWIDE, 255, _vm->_speechFontId);
	}

	FrameHeader frame_head;
	SpriteInfo msgSprite;
	byte *msgSurface;

	frame_head.read(data);

	msgSprite.x = _vm->_screen->getScreenWide() / 2 - frame_head.width / 2;
	msgSprite.y = MENUDEEP / 2 - frame_head.height / 2;
	msgSprite.w = frame_head.width;
	msgSprite.h = frame_head.height;
	msgSprite.type = RDSPR_NOCOMPRESSION;
	msgSprite.data = data + FrameHeader::size();

	_vm->_screen->createSurface(&msgSprite, &msgSurface);
	_vm->_screen->drawSurface(&msgSprite, msgSurface);
	_vm->_screen->deleteSurface(msgSurface);

	free(data);

	// In case the cutscene has a long lead-in, start just before the first
	// line of text.

	frameCounter = text[0]->startFrame - 12;

	// Fake a palette that will hopefully make the text visible. In the
	// opening cutscene it seems to use colours 1 (black) and 255 (white).

	memcpy(oldPal, _vm->_screen->getPalette(), sizeof(oldPal));
	memset(tmpPal, 0, sizeof(tmpPal));
	tmpPal[255 * 4 + 0] = 255;
	tmpPal[255 * 4 + 1] = 255;
	tmpPal[255 * 4 + 2] = 255;
	_vm->_screen->setPalette(0, 256, tmpPal, RDPAL_INSTANT);

	Audio::SoundHandle handle;

	bool skipCutscene = false;

	uint32 flags = Audio::Mixer::FLAG_16BITS;

#ifndef SCUMM_BIG_ENDIAN
	flags |= Audio::Mixer::FLAG_LITTLE_ENDIAN;
#endif

	while (1) {
		if (!text[textCounter])
			break;

		if (frameCounter == text[textCounter]->startFrame) {
			_vm->_screen->clearScene();
			openTextObject(text[textCounter]);
			drawTextObject(NULL, text[textCounter]);
			if (text[textCounter]->speech) {
				_snd->playRaw(&handle, text[textCounter]->speech, text[textCounter]->speechBufferSize, 22050, flags);
			}
		}

		if (frameCounter == text[textCounter]->endFrame) {
			closeTextObject(text[textCounter]);
			_vm->_screen->clearScene();
			_vm->_screen->setNeedFullRedraw();
			textCounter++;
		}

		frameCounter++;
		_vm->_screen->updateDisplay();

		KeyboardEvent *ke = _vm->keyboardEvent();

		if ((ke && ke->keycode == 27) || _vm->_quit) {
			_snd->stopHandle(handle);
			skipCutscene = true;
			break;
		}

		// Simulate ~12 frames per second. I don't know what frame rate
		// the original movies had, or even if it was constant, but
		// this seems to work reasonably.

		_sys->delayMillis(90);
	}

	// Wait for the voice to stop playing. This is to make sure that we
	// don't cut off the speech in mid-sentence, and - even more
	// importantly - that we don't free the sound buffer while it's in use.

	while (_snd->isSoundHandleActive(handle)) {
		_vm->_screen->updateDisplay(false);
		_sys->delayMillis(100);
	}

	closeTextObject(text[textCounter]);

	_vm->_screen->clearScene();
	_vm->_screen->setNeedFullRedraw();

	// HACK: Remove the instructions created above
	Common::Rect r;

	memset(_vm->_screen->getScreen(), 0, _vm->_screen->getScreenWide() * MENUDEEP);
	r.left = r.top = 0;
	r.right = _vm->_screen->getScreenWide();
	r.bottom = MENUDEEP;
	_vm->_screen->updateRect(&r);

	// FIXME: For now, only play the lead-out music for cutscenes that have
	// subtitles.

	if (!skipCutscene && leadOut)
		_vm->_sound->playFx(&_leadOutHandle, leadOut, leadOutLen, Audio::Mixer::kMaxChannelVolume, 0, false, Audio::Mixer::kMusicSoundType);

	_vm->_screen->setPalette(0, 256, oldPal, RDPAL_INSTANT);
}

} // End of namespace Sword2
