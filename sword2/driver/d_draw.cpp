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

#include "stdafx.h"
#include "sound/mixer.h"
#include "bs2/driver/driver96.h"
#include "bs2/driver/rdwin.h"
#include "bs2/driver/d_draw.h"
#include "bs2/driver/palette.h"
#include "bs2/driver/render.h"
#include "bs2/header.h"		// HACK: For cutscenes instruction message
#include "bs2/maketext.h"	// HACK: For cutscenes instruction message
#include "bs2/sword2.h"

namespace Sword2 {

byte *lpBackBuffer;

// Game screen metrics
int16 screenDeep;
int16 screenWide;

// Scroll variables.  scrollx and scrolly hold the current scroll position, 

int16 scrollx;
int16 scrolly;

int32 renderCaps = 0;

/**
 * Initialise the display with the sizes passed in.
 * @return RD_OK, or an error code if the display cannot be set up.
 */

int32 InitialiseDisplay(int16 width, int16 height) {
	g_system->init_size(width, height);

	screenWide = width;
	screenDeep = height;

	lpBackBuffer = (byte *) malloc(screenWide * screenDeep);
	if (!lpBackBuffer)
		return RDERR_OUTOFMEMORY;

	return RD_OK;
}

// FIXME: Clean up this mess. I don't want to add any new flags, but some of
// them should be renamed. Or maybe we should abandon the whole renderCaps
// thing and simply check the numeric value of the graphics quality setting
// instead.

// Note that SetTransFx() actually clears a bit. That's intentional.

void SetTransFx(void) {
	renderCaps &= ~RDBLTFX_ALLHARDWARE;
}

void ClearTransFx(void) {
	renderCaps |= RDBLTFX_ALLHARDWARE;
}

/**
 * Sets the edge blend and arithmetic stretching effects.
 */

void SetBltFx(void) {
	renderCaps |= (RDBLTFX_EDGEBLEND | RDBLTFX_ARITHMETICSTRETCH);
}

/**
 * Clears the edge blend and arithmetic stretching effects.
 */

void ClearBltFx(void) { 
	renderCaps &= ~(RDBLTFX_EDGEBLEND | RDBLTFX_ARITHMETICSTRETCH);
}

void SetShadowFx(void) {
	renderCaps |= RDBLTFX_SHADOWBLEND;
}

void ClearShadowFx(void) {
	renderCaps &= ~RDBLTFX_SHADOWBLEND;
}

/**
 * @return the graphics detail setting
 */

int32 GetRenderType(void) {
	if (renderCaps & RDBLTFX_ALLHARDWARE)
		return 0;

	if (renderCaps & (RDBLTFX_EDGEBLEND | RDBLTFX_ARITHMETICSTRETCH))
		return 3;

	if (renderCaps & RDBLTFX_SHADOWBLEND)
		return 2;

	return 1;
}

/**
 * Fill the screen buffer with palette colour zero. Note that it does not
 * touch the menu areas of the screen.
 */

int32 EraseBackBuffer( void ) {
	memset(lpBackBuffer + MENUDEEP * screenWide, 0, screenWide * RENDERDEEP);
	return RD_OK;
}

void MoviePlayer::openTextObject(_movieTextObject *obj) {
	if (obj->textSprite)
		CreateSurface(obj->textSprite, &_textSurface);
}

void MoviePlayer::closeTextObject(_movieTextObject *obj) {
	if (_textSurface) {
		DeleteSurface(_textSurface);
		_textSurface = NULL;
	}
}

void MoviePlayer::drawTextObject(_movieTextObject *obj) {
	if (obj->textSprite && _textSurface)
		DrawSurface(obj->textSprite, _textSurface);
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

		EraseBackBuffer();

		// HACK: Draw instructions
		//
		// I'm using the the menu area, because that's unlikely to be
		// touched by anything else during the cutscene.

		memset(lpBackBuffer, 0, screenWide * MENUDEEP);

		uint8 msg[] = "Cutscene - Press ESC to exit";
		mem *data = fontRenderer.makeTextSprite(msg, 640, 255, g_sword2->_speechFontId);
		_frameHeader *frame = (_frameHeader *) data->ad;
		_spriteInfo msgSprite;
		uint8 *msgSurface;

		msgSprite.x = screenWide / 2 - frame->width / 2;
		msgSprite.y = RDMENU_MENUDEEP / 2 - frame->height / 2;
		msgSprite.w = frame->width;
		msgSprite.h = frame->height;
		msgSprite.type = RDSPR_DISPLAYALIGN | RDSPR_NOCOMPRESSION | RDSPR_TRANS;
		msgSprite.data = data->ad + sizeof(_frameHeader);

		CreateSurface(&msgSprite, &msgSurface);
		DrawSurface(&msgSprite, msgSurface);
		DeleteSurface(msgSurface);
		memory.freeMemory(data);

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

		memcpy(oldPal, palCopy, 1024);
		memset(tmpPal, 0, 1024);
		tmpPal[255 * 4 + 0] = 255;
		tmpPal[255 * 4 + 1] = 255;
		tmpPal[255 * 4 + 2] = 255;
		BS2_SetPalette(0, 256, tmpPal, RDPAL_INSTANT);

		PlayingSoundHandle handle = 0;

		bool skipCutscene = false;

		while (1) {
			if (!text[textCounter])
				break;

			if (frameCounter == text[textCounter]->startFrame) {
				EraseBackBuffer();
				openTextObject(text[textCounter]);
				drawTextObject(text[textCounter]);
				if (text[textCounter]->speech) {
					g_sword2->_mixer->playRaw(&handle, text[textCounter]->speech, text[textCounter]->speechBufferSize, 22050, SoundMixer::FLAG_16BITS);
				}
			}

			if (frameCounter == text[textCounter]->endFrame) {
				closeTextObject(text[textCounter]);
				EraseBackBuffer();
				textCounter++;
			}

			frameCounter++;

			ServiceWindows();

			_keyboardEvent ke;

			if (ReadKey(&ke) == RD_OK && ke.keycode == 27) {
				g_sword2->_mixer->stopHandle(handle);
				skipCutscene = true;
				break;
			}

			// Simulate ~12 frames per second. I don't know what
			// frame rate the original movies had, or even if it
			// was constant, but this seems to work reasonably.

			g_system->delay_msecs(90);
		}

		closeTextObject(text[textCounter]);

		EraseBackBuffer();
		SetNeedRedraw();

		// HACK: Remove the instructions created above
		Common::Rect r;

		memset(lpBackBuffer, 0, screenWide * MENUDEEP);
		r.left = r.top = 0;
		r.right = screenWide;
		r.bottom = MENUDEEP;
		UploadRect(&r);

		// FIXME: For now, only play the lead-out music for cutscenes
		// that have subtitles.

		if (!skipCutscene)
			g_sound->playLeadOut(musicOut);

		BS2_SetPalette(0, 256, oldPal, RDPAL_INSTANT);
	}

	// Lead-in and lead-out music are, as far as I can tell, only used for
	// the animated cut-scenes, so this seems like a good place to close
	// both of them.

	g_sound->closeFx(-1);
	g_sound->closeFx(-2);

	return RD_OK;
}

} // End of namespace Sword2
