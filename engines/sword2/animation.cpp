/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
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


#include "common/config-manager.h"
#include "common/file.h"
#include "common/events.h"
#include "common/system.h"

#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/header.h"
#include "sword2/logic.h"
#include "sword2/maketext.h"
#include "sword2/mouse.h"
#include "sword2/resman.h"
#include "sword2/screen.h"
#include "sword2/sound.h"
#include "sword2/animation.h"

namespace Sword2 {

///////////////////////////////////////////////////////////////////////////////
// Basic movie player
///////////////////////////////////////////////////////////////////////////////

const MovieInfo MoviePlayer::_movies[19] = {
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
	{ "stone1",    34, true  },
	{ "stone2",   282, false },
	{ "stone3",    65, true  },
	{ "demo",      60, false },
	{ "enddemo",  110, false }
};

MoviePlayer::MoviePlayer(Sword2Engine *vm, const char *name) {
	_vm = vm;
	_name = strdup(name);
	_mixer = _vm->_mixer;
	_system = _vm->_system;
	_pauseTicks = 0;
	_textSurface = NULL;
	_bgSoundStream = NULL;
	_ticks = 0;
	_currentFrame = 0;
	_frameBuffer = NULL;
	_frameWidth = 0;
	_frameHeight = 0;
	_frameX = 0;
	_frameY = 0;
	_black = 1;
	_white = 255;
	_numFrames = 0;
	_leadOutFrame = (uint)-1;
	_seamless = false;
	_framesSkipped = 0;
	_forceFrame = false;
	_currentText = 0;
}

MoviePlayer::~MoviePlayer() {
	free(_name);
}

uint32 MoviePlayer::getTick() {
	return _system->getMillis() - _pauseTicks;
}

void MoviePlayer::updatePalette(byte *pal, bool packed) {
	byte palette[4 * 256];
	byte *p = palette;

	uint32 maxWeight = 0;
	uint32 minWeight = 0xFFFFFFFF;

	for (int i = 0; i < 256; i++) {
		int r = *pal++;
		int g = *pal++;
		int b = *pal++;

		if (!packed)
			pal++;

		uint32 weight = 3 * r * r + 6 * g * g + 2 * b * b;

		if (weight >= maxWeight) {
			_white = i;
			maxWeight = weight;
		}

		if (weight <= minWeight) {
			_black = i;
			minWeight = i;
		}

		*p++ = r;
		*p++ = g;
		*p++ = b;
		*p++ = 0;
	}

	_vm->_screen->setPalette(0, 256, palette, RDPAL_INSTANT);
	_forceFrame = true;
}

void MoviePlayer::savePalette() {
	memcpy(_originalPalette, _vm->_screen->getPalette(), sizeof(_originalPalette));
}

void MoviePlayer::restorePalette() {
	_vm->_screen->setPalette(0, 256, _originalPalette, RDPAL_INSTANT);
}

void MoviePlayer::clearFrame() {
	memset(_frameBuffer, 0, _vm->_screen->getScreenWide() * _vm->_screen->getScreenDeep());
}

void MoviePlayer::updateScreen() {
	_system->updateScreen();
}

bool MoviePlayer::checkSkipFrame() {
	if (_forceFrame) {
		_forceFrame = false;
		return false;
	}

	if (_framesSkipped > 10) {
		warning("Forced frame %d to be displayed", _currentFrame);
		_framesSkipped = 0;
		return false;
	}

	if (_bgSoundStream) {
		if ((_mixer->getSoundElapsedTime(_bgSoundHandle) * 12) / 1000 < _currentFrame + 1)
			return false;
	} else {
		if (getTick() <= _ticks)
			return false;
	}

	_framesSkipped++;
	return true;
}

bool MoviePlayer::syncFrame() {
	_ticks += 83;

	if (checkSkipFrame()) {
		warning("Skipped frame %d", _currentFrame);
		return false;
	}

	if (_bgSoundStream) {
		while (_mixer->isSoundHandleActive(_bgSoundHandle) && (_mixer->getSoundElapsedTime(_bgSoundHandle) * 12) / 1000 < _currentFrame) {
			_system->delayMillis(10);
		}

		// In case the background sound ends prematurely, update _ticks
		// so that we can still fall back on the no-sound sync case for
		// the subsequent frames.

		_ticks = getTick();
	} else {
		while (getTick() < _ticks) {
			_system->delayMillis(10);
		}
	}

	return true;
}

void MoviePlayer::drawFrame() {
	int screenWidth = _vm->_screen->getScreenWide();

	_system->copyRectToScreen(_frameBuffer + _frameY * screenWidth + _frameX, screenWidth, _frameX, _frameY, _frameWidth, _frameHeight);
}

void MoviePlayer::openTextObject(SequenceTextInfo *t) {
	// Pull out the text line to get the official text number (for WAV id)

	uint32 res = t->textNumber / SIZE;
	uint32 localText = t->textNumber & 0xffff;

	// Open text resource and get the line

	byte *text = _vm->fetchTextLine(_vm->_resman->openResource(res), localText);

	_textObject.speechId = READ_LE_UINT16(text);

	// Is it speech or subtitles, or both?

	// If we want subtitles, or there was no sound

	if (_vm->getSubtitles() || !_textObject.speechId) {
		_textObject.textMem = _vm->_fontRenderer->makeTextSprite(text + 2, 600, 255, _vm->_speechFontId, 1);
	}

	_vm->_resman->closeResource(res);

	if (_textObject.textMem) {
		FrameHeader frame;

		frame.read(_textObject.textMem);

		_textObject.textSprite.x = 320 - frame.width / 2;
		_textObject.textSprite.y = 440 - frame.height;
		_textObject.textSprite.w = frame.width;
		_textObject.textSprite.h = frame.height;
		_textObject.textSprite.type = RDSPR_DISPLAYALIGN | RDSPR_NOCOMPRESSION;
		_textObject.textSprite.data = _textObject.textMem + FrameHeader::size();
		_vm->_screen->createSurface(&_textObject.textSprite, &_textSurface);
	}
}

void MoviePlayer::closeTextObject() {
	free(_textObject.textMem);
	_textObject.textMem = NULL;

	_textObject.speechId = 0;

	if (_textSurface) {
		_vm->_screen->deleteSurface(_textSurface);
		_textSurface = NULL;
	}
}

void MoviePlayer::calcTextPosition(int &xPos, int &yPos) {
	xPos = 320 - _textObject.textSprite.w / 2;
	yPos = 420 - _textObject.textSprite.h;
}

void MoviePlayer::drawTextObject() {
	if (_textObject.textMem && _textSurface) {
		int screenWidth = _vm->_screen->getScreenWide();
		byte *src = _textObject.textSprite.data;
		uint16 width = _textObject.textSprite.w;
		uint16 height = _textObject.textSprite.h;
		int xPos, yPos;

		calcTextPosition(xPos, yPos);

		byte *dst = _frameBuffer + yPos * screenWidth + xPos;

		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				if (src[x] == 1)
					dst[x] = _black;
				else if (src[x] == 255)
					dst[x] = _white;
			}
			src += width;
			dst += screenWidth;
		}

		if (yPos + height > _frameY + _frameHeight || width > _frameWidth) {
			_system->copyRectToScreen(_frameBuffer + yPos * screenWidth + xPos, screenWidth, xPos, yPos, width, height);
		}
	}
}

void MoviePlayer::undrawTextObject() {
	if (_textObject.textMem) {
		int xPos, yPos;

		calcTextPosition(xPos, yPos);
		uint16 width = _textObject.textSprite.w;
		uint16 height = _textObject.textSprite.h;

		// We only need to undraw the text if it's outside the frame.
		// Otherwise the next frame will cover the old text anyway.

		if (yPos + height > _frameY + _frameHeight || width > _frameWidth) {
			int screenWidth = _vm->_screen->getScreenWide();
			byte *dst = _frameBuffer + yPos * screenWidth + xPos;

			for (int y = 0; y < height; y++) {
				memset(dst, 0, width);
				dst += screenWidth;
			}

			_system->copyRectToScreen(_frameBuffer + yPos * screenWidth + xPos, screenWidth, xPos, yPos, width, height);
		}
	}
}

bool MoviePlayer::load() {
	_bgSoundStream = NULL;
	_currentText = 0;
	_currentFrame = 0;

	for (int i = 0; i < ARRAYSIZE(_movies); i++) {
		if (scumm_stricmp(_name, _movies[i].name) == 0) {
			_seamless = _movies[i].seamless;
			_numFrames = _movies[i].frames;
			if (_numFrames > 60)
				_leadOutFrame = _numFrames - 60;

			// Not all cutscenes cover the entire screen, so clear
			// it. We will always clear the game screen, no matter
			// how the cutscene is to be displayed. (We have to do
			// this before showing the overlay.)

			_vm->_mouse->closeMenuImmediately();

			if (!_seamless) {
				_vm->_screen->clearScene();
			}

			_vm->_screen->updateDisplay();
			return true;
		}
	}

	return false;
}

bool MoviePlayer::userInterrupt() {
	Common::Event event;
	bool terminate = false;

	Common::EventManager *eventMan = _system->getEventManager();
	while (eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_SCREEN_CHANGED:
			handleScreenChanged();
			break;
		case Common::EVENT_RTL:
		case Common::EVENT_QUIT:
			terminate = true;
			break;
		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
				terminate = true;
			break;
		default:
			break;
		}
	}

	return terminate;
}

void MoviePlayer::play(SequenceTextInfo *textList, uint32 numLines, int32 leadIn, int32 leadOut) {
	bool terminate = false;
	bool textVisible = false;
	bool startNextText = false;

	// This happens if the user quits during the "eye" cutscene.
	if (_vm->shouldQuit())
		return;

	_numSpeechLines = numLines;
	_firstSpeechFrame = (numLines > 0) ? textList[0].startFrame : 0;

	if (leadIn) {
		_vm->_sound->playMovieSound(leadIn, kLeadInSound);
	}

	savePalette();

	_framesSkipped = 0;
	_ticks = getTick();
	_bgSoundStream = Audio::AudioStream::openStreamFile(_name);

	if (_bgSoundStream) {
		_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_bgSoundHandle, _bgSoundStream);
	}

	while (!terminate && _currentFrame < _numFrames && decodeFrame()) {
		_currentFrame++;

		// The frame has been decoded. Now draw the subtitles, if any,
		// before drawing it to the screen.

		if (_currentText < numLines) {
			SequenceTextInfo *t = &textList[_currentText];

			if (_currentFrame == t->startFrame) {
				openTextObject(t);
				textVisible = true;

				if (_textObject.speechId) {
					startNextText = true;
				}
			}

			if (startNextText && _vm->_sound->amISpeaking() == RDSE_QUIET) {
				_vm->_sound->playCompSpeech(_textObject.speechId, 16, 0);
				startNextText = false;
			}

			if (_currentFrame == t->endFrame) {
				undrawTextObject();
				closeTextObject();
				_currentText++;
				textVisible = false;
			}

			if (textVisible)
				drawTextObject();
		}

		if (leadOut && _currentFrame == _leadOutFrame) {
			_vm->_sound->playMovieSound(leadOut, kLeadOutSound);
		}

		if (syncFrame()) {
			drawFrame();
			updateScreen();
		}

		if (userInterrupt()) {
			terminate = true;
		}
	}

	if (!_seamless) {
		// Most cutscenes fade to black on their own, but not all of
		// them. I think it looks better if they do.

		clearFrame();

		// If the sound is still playing, draw the subtitles one final
		// time. This happens in the "carib" cutscene.

		if (textVisible && _vm->_sound->amISpeaking() == RDSE_SPEAKING) {
			drawTextObject();
		}

		drawFrame();
		updateScreen();
	}

	if (!terminate) {
		// Wait for the voice and sound track to stop playing. This is
		// to make sure that we don't cut off the speech in
		// mid-sentence, and - even more importantly - that we don't
		// free the sound buffer while it's still in use.

		while (_vm->_sound->amISpeaking() == RDSE_SPEAKING || _mixer->isSoundHandleActive(_bgSoundHandle)) {
			if (userInterrupt()) {
				terminate = true;
				_vm->_sound->stopSpeech();
				_mixer->stopHandle(_bgSoundHandle);
			}
			_system->delayMillis(100);
		}
	} else {
		_vm->_sound->stopSpeech();
		_mixer->stopHandle(_bgSoundHandle);
	}

	// The current text object may still be open
	undrawTextObject();
	closeTextObject();

	if (!_seamless) {
		clearFrame();
		drawFrame();
		updateScreen();
	}

	// Setting the palette implies a full redraw.
	restorePalette();
}

void MoviePlayer::pauseMovie(bool pause) {
	_mixer->pauseHandle(_bgSoundHandle, pause);

	if (pause) {
		_pauseStartTick = _system->getMillis();
	} else {
		_pauseTicks += (_system->getMillis() - _pauseStartTick);
	}
}

#ifdef USE_ZLIB

///////////////////////////////////////////////////////////////////////////////
// Movie player for the new DXA movies
///////////////////////////////////////////////////////////////////////////////

MoviePlayerDXA::MoviePlayerDXA(Sword2Engine *vm, const char *name)
	: MoviePlayer(vm, name) {
	debug(0, "Creating DXA cutscene player");
}

MoviePlayerDXA::~MoviePlayerDXA() {
	closeFile();
}

void MoviePlayerDXA::setPalette(byte *pal) {
	updatePalette(pal);
}

bool MoviePlayerDXA::decodeFrame() {
	decodeNextFrame();
	copyFrameToBuffer(_frameBuffer, _frameX, _frameY, _vm->_screen->getScreenWide());
	return true;
}

bool MoviePlayerDXA::load() {
	if (!MoviePlayer::load())
		return false;

	char filename[20];

	snprintf(filename, sizeof(filename), "%s.dxa", _name);

	if (loadFile(filename)) {
		// The Broken Sword games always use external audio tracks.
		if (_fd->readUint32BE() != MKID_BE('NULL'))
			return false;

		_frameBuffer = _vm->_screen->getScreen();

		_frameWidth = getWidth();
		_frameHeight = getHeight();

		_frameX = (_vm->_screen->getScreenWide() - _frameWidth) / 2;
		_frameY = (_vm->_screen->getScreenDeep() - _frameHeight) / 2;

		return true;
	}

	return false;
}

#endif

#ifdef USE_MPEG2

///////////////////////////////////////////////////////////////////////////////
// Movie player for the old MPEG movies
///////////////////////////////////////////////////////////////////////////////

MoviePlayerMPEG::MoviePlayerMPEG(Sword2Engine *vm, const char *name)
	: MoviePlayer(vm, name) {
#ifdef BACKEND_8BIT
	debug(0, "Creating MPEG cutscene player (8-bit)");
#else
	debug(0, "Creating MPEG cutscene player (16-bit)");
#endif
}

MoviePlayerMPEG::~MoviePlayerMPEG() {
	delete _anim;
	_anim = NULL;
}

bool MoviePlayerMPEG::load() {
	if (!MoviePlayer::load())
		return false;

	_anim = new AnimationState(_vm, this);

	if (!_anim->init(_name)) {
		delete _anim;
		_anim = NULL;
		return false;
	}

#ifdef BACKEND_8BIT
	_frameBuffer = _vm->_screen->getScreen();
#endif

	return true;
}

bool MoviePlayerMPEG::decodeFrame() {
	bool result = _anim->decodeFrame();

#ifdef BACKEND_8BIT
	_frameWidth = _anim->getFrameWidth();
	_frameHeight = _anim->getFrameHeight();

	_frameX = (_vm->_screen->getScreenWide() - _frameWidth) / 2;
	_frameY = (_vm->_screen->getScreenDeep() - _frameHeight) / 2;
#endif

	return result;
}

AnimationState::AnimationState(Sword2Engine *vm, MoviePlayer *player)
	: BaseAnimationState(vm->_system, 640, 480) {
	_vm = vm;
	_player = player;
}

AnimationState::~AnimationState() {
}

#ifdef BACKEND_8BIT

void AnimationState::setPalette(byte *pal) {
	_player->updatePalette(pal, false);
}

#else

void MoviePlayerMPEG::handleScreenChanged() {
	_anim->handleScreenChanged();
}

void MoviePlayerMPEG::clearFrame() {
	_anim->clearFrame();
}

void MoviePlayerMPEG::drawFrame() {
}

void MoviePlayerMPEG::updateScreen() {
	_anim->updateScreen();
}

void MoviePlayerMPEG::drawTextObject() {
	if (_textObject.textMem && _textSurface) {
		_anim->drawTextObject(&_textObject.textSprite, _textSurface);
	}
}

void MoviePlayerMPEG::undrawTextObject() {
	// As long as we only have subtitles for full-sized cutscenes, we don't
	// really need to implement this function.
}

void AnimationState::drawTextObject(SpriteInfo *s, byte *src) {
	int moviePitch = _movieScale * _movieWidth;
	int textX = _movieScale * s->x;
	int textY = _movieScale * (_frameHeight - s->h - 12);

	OverlayColor *dst = _overlay + textY * moviePitch + textX;

	Graphics::PixelFormat format = _sys->getOverlayFormat();
	OverlayColor pen = Graphics::RGBToColor(255, 255, 255, format);
	OverlayColor border = Graphics::RGBToColor(0, 0, 0, format);

	// TODO: Use the AdvMame scalers for the text? Pre-scale it?

	for (int y = 0; y < s->h; y++) {
		OverlayColor *ptr = dst;

		for (int x = 0; x < s->w; x++) {
			switch (src[x]) {
			case 1:
				*ptr++ = border;
				if (_movieScale > 1) {
					*ptr++ = border;
					if (_movieScale > 2)
						*ptr++ = border;
				}
				break;
			case 255:
				*ptr++ = pen;
				if (_movieScale > 1) {
					*ptr++ = pen;
					if (_movieScale > 2)
						*ptr++ = pen;
				}
				break;
			default:
				ptr += _movieScale;
				break;
			}
		}

		if (_movieScale > 1) {
			memcpy(dst + moviePitch, dst, _movieScale * s->w * sizeof(OverlayColor));
			if (_movieScale > 2)
				memcpy(dst + 2 * moviePitch, dst, _movieScale * s->w * sizeof(OverlayColor));
		}

		dst += _movieScale * moviePitch;
		src += s->w;
	}
}
#endif

void AnimationState::clearFrame() {
#ifdef BACKEND_8BIT
	memset(_vm->_screen->getScreen(), 0, _movieWidth * _movieHeight);
#else
	Graphics::PixelFormat format = _sys->getOverlayFormat();
	OverlayColor black = Graphics::RGBToColor(0, 0, 0, format);

	for (int i = 0; i < _movieScale * _movieWidth * _movieScale * _movieHeight; i++)
		_overlay[i] = black;
#endif
}

void AnimationState::drawYUV(int width, int height, byte *const *dat) {
	_frameWidth = width;
	_frameHeight = height;

#ifdef BACKEND_8BIT
	byte *buf = _vm->_screen->getScreen() + ((480 - height) / 2) * RENDERWIDE + (640 - width) / 2;

	int x, y;

	int ypos = 0;
	int cpos = 0;
	int linepos = 0;

	for (y = 0; y < height; y += 2) {
		for (x = 0; x < width; x += 2) {
			int i = ((((dat[2][cpos] + ROUNDADD) >> SHIFT) * (BITDEPTH + 1)) + ((dat[1][cpos] + ROUNDADD) >> SHIFT)) * (BITDEPTH + 1);
			cpos++;

			buf[linepos               ] = _lut[i + ((dat[0][        ypos  ] + ROUNDADD) >> SHIFT)];
			buf[RENDERWIDE + linepos++] = _lut[i + ((dat[0][width + ypos++] + ROUNDADD) >> SHIFT)];
			buf[linepos               ] = _lut[i + ((dat[0][        ypos  ] + ROUNDADD) >> SHIFT)];
			buf[RENDERWIDE + linepos++] = _lut[i + ((dat[0][width + ypos++] + ROUNDADD) >> SHIFT)];
		}
		linepos += (2 * RENDERWIDE - width);
		ypos += width;
	}
#else
	plotYUV(width, height, dat);
#endif
}

#endif

///////////////////////////////////////////////////////////////////////////////
// Dummy player for subtitled speech only
///////////////////////////////////////////////////////////////////////////////

MoviePlayerDummy::MoviePlayerDummy(Sword2Engine *vm, const char *name)
	: MoviePlayer(vm, name) {
	debug(0, "Creating Dummy cutscene player");
}

MoviePlayerDummy::~MoviePlayerDummy() {
}

bool MoviePlayerDummy::load() {
	if (!MoviePlayer::load())
		return false;

	_frameBuffer = _vm->_screen->getScreen();

	_frameWidth = 640;
	_frameHeight = 400;
	_frameX = 0;
	_frameY = 40;

	return true;
}

bool MoviePlayerDummy::decodeFrame() {
	if ((_currentFrame == 0 && _numSpeechLines > 0) || _mixer->isSoundHandleActive(_bgSoundHandle)) {
		byte dummyPalette[] = {
			  0,   0,   0, 0,
			255, 255, 255, 0,
		};

		// 0 is always black
		// 1 is the border colour - black
		// 255 is the pen colour - white

		_system->setPalette(dummyPalette, 0, 1);
		_system->setPalette(dummyPalette, 1, 1);
		_system->setPalette(dummyPalette + 4, 255, 1);

		byte msgNoCutscenesRU[] = "Po\344uk - to\344\345ko pev\345: hagmute k\344abuwy Ucke\343n, u\344u nocetute ca\343t npoekta u ckava\343te budeo po\344uku";

#if defined(USE_MPEG2) || defined(USE_ZLIB)
		byte msgNoCutscenes[] = "Cutscene - Narration Only: Press ESC to exit, or visit www.scummvm.org to download cutscene videos";
#else
		byte msgNoCutscenes[] = "Cutscene - Narration Only: Press ESC to exit, or recompile ScummVM with MPEG2 or ZLib support";
#endif

		byte *msg;

		// Russian version substituted latin characters with Cyrillic.
		if (Common::parseLanguage(ConfMan.get("language")) == Common::RU_RUS) {
			msg = msgNoCutscenesRU;
		} else {
			msg = msgNoCutscenes;
		}

		byte *data = _vm->_fontRenderer->makeTextSprite(msg, RENDERWIDE, 255, _vm->_speechFontId);

		FrameHeader frame_head;
		SpriteInfo msgSprite;
		byte *msgSurface;

		frame_head.read(data);

		msgSprite.x = _vm->_screen->getScreenWide() / 2 - frame_head.width / 2;
		msgSprite.y = (480 - frame_head.height) / 2;
		msgSprite.w = frame_head.width;
		msgSprite.h = frame_head.height;
		msgSprite.type = RDSPR_NOCOMPRESSION;
		msgSprite.data = data + FrameHeader::size();

		_vm->_screen->createSurface(&msgSprite, &msgSurface);
		_vm->_screen->drawSurface(&msgSprite, msgSurface);
		_vm->_screen->deleteSurface(msgSurface);

		free(data);
		updateScreen();
	}

	// If we have played the final voice-over, skip ahead to the lead out

	if (!_mixer->isSoundHandleActive(_bgSoundHandle) &&
	    _currentText >= _numSpeechLines &&
	    _vm->_sound->amISpeaking() == RDSE_QUIET &&
	    _leadOutFrame != (uint)-1 &&
	    _currentFrame < _leadOutFrame) {
		_currentFrame = _leadOutFrame - 1;
	}

	return true;
}

bool MoviePlayerDummy::syncFrame() {
	if ((_numSpeechLines == 0 || _currentFrame < _firstSpeechFrame) && !_mixer->isSoundHandleActive(_bgSoundHandle)) {
		_ticks = getTick();
		return false;
	}

	return MoviePlayer::syncFrame();
}

void MoviePlayerDummy::drawFrame() {
}

void MoviePlayerDummy::drawTextObject() {
	if (_textObject.textMem && _textSurface) {
		_vm->_screen->drawSurface(&_textObject.textSprite, _textSurface);
	}
}

void MoviePlayerDummy::undrawTextObject() {
	if (_textObject.textMem && _textSurface) {
		memset(_textSurface, 1, _textObject.textSprite.w * _textObject.textSprite.h);
		drawTextObject();
	}
}

///////////////////////////////////////////////////////////////////////////////
// Factory function for creating the appropriate cutscene player
///////////////////////////////////////////////////////////////////////////////

MoviePlayer *makeMoviePlayer(Sword2Engine *vm, const char *name) {
	static char filename[20];

#ifdef USE_ZLIB
	snprintf(filename, sizeof(filename), "%s.dxa", name);

	if (Common::File::exists(filename)) {
		return new MoviePlayerDXA(vm, name);
	}
#endif

#ifdef USE_MPEG2
	snprintf(filename, sizeof(filename), "%s.mp2", name);

	if (Common::File::exists(filename)) {
		return new MoviePlayerMPEG(vm, name);
	}
#endif

	return new MoviePlayerDummy(vm, name);
}

} // End of namespace Sword2
