/* Copyright (C) 1994-2004 Revolution Software Ltd
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "common/stdafx.h"
#include "sword2/sword2.h"
#include "sword2/driver/menu.h"
#include "sword2/driver/render.h"

namespace Sword2 {

Graphics::Graphics(Sword2Engine *vm, int16 width, int16 height) 
	: _vm(vm), _iconCount(0), _needFullRedraw(false),
	  _fadeStatus(RDFADE_NONE), _mouseSprite(NULL), _mouseAnim(NULL),
	  _luggageAnim(NULL), _layer(0), _renderAverageTime(60),
	  _lightMask(NULL), _screenWide(width), _screenDeep(height) {

	int i, j;

	_buffer = _dirtyGrid = NULL;

	_buffer = (byte *) malloc(width * height);
	if (!_buffer)
		error("Could not initialise display");

	_vm->_system->init_size(width, height);

	_gridWide = width / CELLWIDE;
	_gridDeep = height / CELLDEEP;

	if ((width % CELLWIDE) || (height % CELLDEEP))
		error("Bad cell size");

	_dirtyGrid = (byte *) calloc(_gridWide, _gridDeep);
	if (!_buffer)
		error("Could not initialise dirty grid");

	for (i = 0; i < ARRAYSIZE(_blockSurfaces); i++)
		_blockSurfaces[i] = NULL;

	for (i = 0; i < 2; i++) {
		for (j = 0; j < RDMENU_MAXPOCKETS; j++) {
			_icons[i][j] = NULL;
			_pocketStatus[i][j] = 0;
		}

		_menuStatus[i] = RDMENU_HIDDEN;
	}
}

Graphics::~Graphics() {
	free(_buffer);
	free(_dirtyGrid);
	closeBackgroundLayer();
	free(_lightMask);
	free(_mouseAnim);
	free(_luggageAnim);
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < RDMENU_MAXPOCKETS; j++)
			free(_icons[i][j]);
}

/**
 * @return the graphics detail setting
 */

int8 Graphics::getRenderLevel(void) {
	return _renderLevel;
}

void Graphics::setRenderLevel(int8 level) {
	_renderLevel = level;

	switch (_renderLevel) {
	case 0:
		// Lowest setting: no fancy stuff
		_renderCaps = 0;
		break;
	case 1:
		// Medium-low setting: transparency-blending
		_renderCaps = RDBLTFX_SPRITEBLEND;
		break;
	case 2:
		// Medium-high setting: transparency-blending + shading
		_renderCaps = RDBLTFX_SPRITEBLEND | RDBLTFX_SHADOWBLEND;
		break;
	case 3:
		// Highest setting: transparency-blending + shading +
		// edge-blending + improved stretching
		_renderCaps = RDBLTFX_SPRITEBLEND | RDBLTFX_SHADOWBLEND | RDBLTFX_EDGEBLEND;
		break;
	}
}

/**
 * Fill the screen buffer with palette colour zero. Note that it does not
 * touch the menu areas of the screen.
 */

void Graphics::clearScene(void) {
	memset(_buffer + MENUDEEP * _screenWide, 0, _screenWide * RENDERDEEP);
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

void MoviePlayer::drawTextObject(MovieTextObject *obj) {
	if (obj->textSprite && _textSurface)
		_vm->_graphics->drawSurface(obj->textSprite, _textSurface);
}

/**
 * Plays an animated cutscene.
 * @param filename the file name of the cutscene file
 * @param text the subtitles and voiceovers for the cutscene
 * @param musicOut lead-out music
 */

int32 MoviePlayer::play(const char *filename, MovieTextObject *text[], uint8 *musicOut) {
#ifdef USE_MPEG2
	int frameCounter = 0, textCounter = 0;
	PlayingSoundHandle handle;
	bool skipCutscene = false, textVisible = false;
	uint32 flags = SoundMixer::FLAG_16BITS, ticks = _vm->_system->get_msecs() + 83;

	uint8 oldPal[1024];
	memcpy(oldPal, _vm->_graphics->_palCopy, 1024);

	AnimationState * anim = initAnimation(filename);
	if (!anim) {
		// Missing Files? Use the old 'Narration Only' hack
		playDummy(filename, text, musicOut);
		return RD_OK;
	}

	_vm->_graphics->clearScene();
	memset(_vm->_graphics->_buffer, 0, _vm->_graphics->_screenWide * MENUDEEP);


#ifndef SCUMM_BIG_ENDIAN
	flags |= SoundMixer::FLAG_LITTLE_ENDIAN;
#endif

	while (1) {
		if (!decodeFrame(anim)) break;
		_vm->_graphics->setNeedFullRedraw();

		if (text && text[textCounter]) {                      
			if (frameCounter == text[textCounter]->startFrame) {
				openTextObject(text[textCounter]);
				textVisible = true;
				if (text[textCounter]->speech) {
					_vm->_mixer->playRaw(&handle, text[textCounter]->speech, text[textCounter]->speechBufferSize, 22050, flags);
				}
			}

			if (frameCounter == text[textCounter]->endFrame) {
				closeTextObject(text[textCounter]);
				textCounter++;
				textVisible = false;
			}
			if (textVisible)
				drawTextObject(text[textCounter]);
		}

		frameCounter++;

		_vm->_graphics->updateDisplay(true);

		KeyboardEvent ke;

		if ((_vm->_input->readKey(&ke) == RD_OK && ke.keycode == 27) || _vm->_quit) {
			_vm->_mixer->stopHandle(handle);
			skipCutscene = true;
			break;
		}

		// Simulate ~12 frames per second. I don't know what
		// frame rate the original movies had, or even if it
		// was constant, but this seems to work reasonably.

		_vm->sleepUntil(ticks);
		ticks += 82;

	}

	// Wait for the voice to stop playing. This is to make sure
	// that we don't cut off the speech in mid-sentence, and - even
	// more importantly - that we don't free the sound buffer while
	// it's in use.

	while (handle.isActive()) {
		_vm->_graphics->updateDisplay(false);
		_vm->_system->delay_msecs(100);
	}

	if (text)
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
	if (!skipCutscene)
		_vm->_sound->playLeadOut(musicOut);

	_vm->_graphics->setPalette(0, 256, oldPal, RDPAL_INSTANT);

	doneAnimation(anim);

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

// This just plays the cutscene with voiceovers / subtitles, in case the files are missing
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
		Memory *data = _vm->_fontRenderer->makeTextSprite(msg, 640, 255, _vm->_speechFontId);
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
		//
		// The text should probably be colored the same as the rest of
		// the in-game text.

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
				drawTextObject(text[textCounter]);
				if (text[textCounter]->speech) {
					_vm->_mixer->playRaw(&handle, text[textCounter]->speech, text[textCounter]->speechBufferSize, 22050, flags);
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
				_vm->_mixer->stopHandle(handle);
				skipCutscene = true;
				break;
			}

			// Simulate ~12 frames per second. I don't know what
			// frame rate the original movies had, or even if it
			// was constant, but this seems to work reasonably.

			_vm->_system->delay_msecs(90);
		}

		// Wait for the voice to stop playing. This is to make sure
		// that we don't cut off the speech in mid-sentence, and - even
		// more importantly - that we don't free the sound buffer while
		// it's in use.

		while (handle.isActive()) {
			_vm->_graphics->updateDisplay(false);
			_vm->_system->delay_msecs(100);
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

		if (!skipCutscene)
			_vm->_sound->playLeadOut(musicOut);

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
