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
#include "sword2/sword2.h"
#include "sword2/driver/menu.h"
#include "sword2/driver/render.h"

#include "common/file.h"

namespace Sword2 {

/**
 * Build 'Best-Match' RGB lookup table
 */

void AnimationState::buildLookup(int p, int lines) {
	int y, cb;
	int r, g, b, ii;
  
	if (p != curpal) {
		curpal = p;
		cr = 0;
		pos = 0;
	}

	if (cr >= BITDEPTH)
		return;

	for (ii = 0; ii < lines; ii++) {
		r = (-16 * 256 + (int) (256 * 1.596) * ((cr << SHIFT) - 128)) / 256;
		for (cb = 0; cb < BITDEPTH; cb++) {
			g = (-16 * 256 - (int) (0.813 * 256) * ((cr << SHIFT) - 128) - (int) (0.391 * 256) * ((cb << SHIFT) - 128)) / 256;
			b = (-16 * 256 + (int) (2.018 * 256) * ((cb << SHIFT) - 128)) / 256;

			for (y = 0; y < BITDEPTH; y++) {
				int idx, bst = 0;
				int dis = 2 * SQR(r - palettes[p].pal[0]) + 4 * SQR(g - palettes[p].pal[1]) + SQR(b - palettes[p].pal[2]);

				for (idx = 1; idx < 256; idx++) {
					long d2 = 2 * SQR(r - palettes[p].pal[4 * idx]) + 4 * SQR(g - palettes[p].pal[4 * idx + 1]) + SQR(b - palettes[p].pal[4 * idx + 2]);
					if (d2 < dis) {
						bst = idx;
						dis = d2;
					}
				}
				lut2[pos++] = bst;
	
				r += (1 << SHIFT);
				g += (1 << SHIFT);
				b += (1 << SHIFT);
			}
			r -= 256;
		}
		cr++;
		if (cr >= BITDEPTH)
			return;
	}
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

void MoviePlayer::checkPaletteSwitch(AnimationState * st) {
	// if we have reached the last image with this palette, switch to new one
	if (st->framenum == st->palettes[st->palnum].end) {
		unsigned char *l = st->lut2;
		st->palnum++;
		_vm->_graphics->setPalette(0, 256, st->palettes[st->palnum].pal, RDPAL_INSTANT);
		st->lutcalcnum =  (BITDEPTH + st->palettes[st->palnum].end - (st->framenum + 1) + 2) / (st->palettes[st->palnum].end - (st->framenum + 1) + 2);
		st->lut2 = st->lut;
		st->lut = l;
	}
}

#ifdef USE_MPEG2

bool MoviePlayer::decodeFrame(AnimationState * st) {
	mpeg2_state_t state;
	const mpeg2_sequence_t *sequence_i;
	size_t size = (size_t) -1;

	do {
		state = mpeg2_parse(st->decoder);
		sequence_i = st->info->sequence;

		switch (state) {
		case STATE_BUFFER:
			size = st->mpgfile->read(st->buffer, BUFFER_SIZE);
			mpeg2_buffer(st->decoder, st->buffer, st->buffer + size);
			break;

		case STATE_SLICE:
		case STATE_END:
			if (st->info->display_fbuf) {
				checkPaletteSwitch(st);
				_vm->_graphics->plotYUV(st->lut, sequence_i->width, sequence_i->height, st->info->display_fbuf->buf);
				st->framenum++;
				st->buildLookup(st->palnum + 1, st->lutcalcnum);
				return true;
			}
			break;

		default:
			break;
		}
	} while (size);

	return false;
}

AnimationState *MoviePlayer::initAnimation(const char *name) {
	char basename[512], tempFile[512];
	AnimationState *st = new AnimationState;
  	int i, p;

	strcpy(basename, name);
	basename[strlen(basename)-4] = 0;

	// Load lookup palettes
	// TODO: Binary format so we can use File class
	sprintf(tempFile, "%s/%s.pal", _vm->getGameDataPath(), basename);
	FILE *f = fopen(tempFile, "r");

	if (!f) {
		warning("Cutscene: %s.pal palette missing", basename);
		return 0;
	}

	p = 0;
	while (!feof(f)) {
		fscanf(f, "%i %i", &st->palettes[p].end, &st->palettes[p].cnt);
  		for (i = 0; i < st->palettes[p].cnt; i++) {
  			int r, g, b;
  			fscanf(f, "%i", &r);
  			fscanf(f, "%i", &g);
  			fscanf(f, "%i", &b);
			st->palettes[p].pal[4 * i] = r;
			st->palettes[p].pal[4 * i + 1] = g;
			st->palettes[p].pal[4 * i + 2] = b;
		}
		p++;
	}
	fclose(f);

	st->palnum = 0;
	_vm->_graphics->setPalette(0, 256, st->palettes[st->palnum].pal, RDPAL_INSTANT);
	st->lut = st->lut2 = st->lookup[0];
	st->curpal = -1;
	st->cr = 0;
	st->buildLookup(st->palnum, 256);
	st->lut2 = st->lookup[1];

	// Open MPEG2 stream
	st->mpgfile = new File();
	sprintf(tempFile, "%s.mp2", basename);
	if (!st->mpgfile->open(tempFile)) {
		warning("Cutscene: Could not open %s", tempFile);
		delete st;
		return 0;
	}

	// Load and configure decoder
	st->decoder = mpeg2_init();
	if (st->decoder == NULL) {
		warning("Cutscene: Could not allocate an MPEG2 decoder");
		delete st;
		return 0;
	}

	st->info = mpeg2_info(st->decoder);
	st->framenum = 0;

	// Load in palette data
	st->lutcalcnum = (BITDEPTH + st->palettes[st->palnum].end + 2) / (st->palettes[st->palnum].end + 2);

	/* Play audio - TODO: Sync with video?*/

#ifdef USE_VORBIS
	// Another TODO: There is no reason that this only allows OGG, and not MP3, or any other format
	// the mixer might support one day... is there?
	File *sndFile = new File;
	sprintf(tempFile, "%s.ogg", basename);
	if (sndFile->open(tempFile))
		_vm->_mixer->playVorbis(&st->bgSound, sndFile, 100000000);
#endif

	// FIXME: This leaks (sndFile will never be deleted)

	return st;
}

void MoviePlayer::doneAnimation(AnimationState *st) {
	if (st->bgSound.isActive())
		_vm->_mixer->stopHandle(st->bgSound);

	mpeg2_close (st->decoder);
	st->mpgfile->close();
	delete st->mpgfile;
	delete st;
}

#else

bool MoviePlayer::decodeFrame(AnimationState * st) {
	// Dummy for MPEG2-less builds
	return false;
}

AnimationState *MoviePlayer::initAnimation(const char *name) {
	return 0;
}

void MoviePlayer::doneAnimation(AnimationState *st) {
}

#endif

} // End of namespace Sword2
