/* Copyright (C) 1994-2003 Revolution Software Ltd
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

	_buffer = (byte *) malloc(width * height);
	if (!_buffer)
		error("Could not initialise display");

	_vm->_system->init_size(width, height);

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

void MoviePlayer::openTextObject(_movieTextObject *obj) {
	if (obj->textSprite)
		_vm->_graphics->createSurface(obj->textSprite, &_textSurface);
}

void MoviePlayer::closeTextObject(_movieTextObject *obj) {
	if (_textSurface) {
		_vm->_graphics->deleteSurface(_textSurface);
		_textSurface = NULL;
	}
}

void MoviePlayer::drawTextObject(_movieTextObject *obj) {
	if (obj->textSprite && _textSurface)
		_vm->_graphics->drawSurface(obj->textSprite, _textSurface);
}

/**
 * Plays an animated cutscene.
 * @param filename the file name of the cutscene file
 * @param text the subtitles and voiceovers for the cutscene
 * @param musicOut lead-out music
 */

int32 MoviePlayer::play(char *filename, _movieTextObject *text[], uint8 *musicOut) {
	warning("semi-stub PlaySmacker %s", filename);

	// WORKAROUND: For now, we just do the voice-over parts of the
	// movies, since they're separate from the actual smacker files.

	if (text) {
		uint8 oldPal[1024];
		uint8 tmpPal[1024];

		_vm->_graphics->clearScene();

		// HACK: Draw instructions
		//
		// I'm using the the menu area, because that's unlikely to be
		// touched by anything else during the cutscene.

		memset(_vm->_graphics->_buffer, 0, _vm->_graphics->_screenWide * MENUDEEP);

		uint8 msg[] = "Cutscene - Press ESC to exit";
		mem *data = _vm->_fontRenderer->makeTextSprite(msg, 640, 255, _vm->_speechFontId);
		_frameHeader *frame = (_frameHeader *) data->ad;
		_spriteInfo msgSprite;
		uint8 *msgSurface;

		msgSprite.x = _vm->_graphics->_screenWide / 2 - frame->width / 2;
		msgSprite.y = RDMENU_MENUDEEP / 2 - frame->height / 2;
		msgSprite.w = frame->width;
		msgSprite.h = frame->height;
		msgSprite.type = RDSPR_NOCOMPRESSION;
		msgSprite.data = data->ad + sizeof(_frameHeader);

		_vm->_graphics->createSurface(&msgSprite, &msgSurface);
		_vm->_graphics->drawSurface(&msgSprite, msgSurface);
		_vm->_graphics->deleteSurface(msgSurface);
		_vm->_memory->freeMemory(data);

		// In case the cutscene has a long lead-in, start just before
		// the first line of text.

		int frameCounter = text[0]->startFrame - 12;
		int textCounter = 0;

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
				textCounter++;
			}

			frameCounter++;

			_vm->_graphics->updateDisplay();

			_keyboardEvent ke;

			if (_vm->_input->readKey(&ke) == RD_OK && ke.keycode == 27) {
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
			_vm->_system->delay_msecs(100);
		};

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
