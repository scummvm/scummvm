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
#include "sword2/driver/animation.h"
#include "sword2/driver/menu.h"
#include "sword2/driver/render.h"

#include "common/file.h"

#include "sound/vorbis.h"

namespace Sword2 {

AnimationState::AnimationState(Sword2Engine *vm)
	: _vm(vm) {
}

AnimationState::~AnimationState() {
#ifdef USE_MPEG2
	_vm->_mixer->stopHandle(bgSound);
	if (decoder)
		mpeg2_close(decoder);
	delete mpgfile;
        delete sndfile;
#ifndef BACKEND_8BIT
        _vm->_system->hide_overlay();
        delete overlay;
#endif
#endif
}

bool AnimationState::init(const char *name) {
#ifdef USE_MPEG2
	char tempFile[512];

	decoder = NULL;
	mpgfile = NULL;
	sndfile = NULL;
        bgSoundStream = NULL;

#ifdef BACKEND_8BIT

        int i, p;

	// Load lookup palettes
	// TODO: Binary format so we can use File class
	sprintf(tempFile, "%s/%s.pal", _vm->getGameDataPath(), name);
	FILE *f = fopen(tempFile, "r");

	if (!f) {
		warning("Cutscene: %s.pal palette missing", name);
		return false;
	}

	p = 0;
	while (!feof(f)) {
		if (fscanf(f, "%i %i", &palettes[p].end, &palettes[p].cnt) != 2)
			break;
		for (i = 0; i < palettes[p].cnt; i++) {
			int r, g, b;
			fscanf(f, "%i", &r);
			fscanf(f, "%i", &g);
			fscanf(f, "%i", &b);
			palettes[p].pal[4 * i] = r;
			palettes[p].pal[4 * i + 1] = g;
			palettes[p].pal[4 * i + 2] = b;
			palettes[p].pal[4 * i + 3] = 0;
		}
		for (; i < 256; i++) {
			palettes[p].pal[4 * i] = 0;
			palettes[p].pal[4 * i + 1] = 0;
			palettes[p].pal[4 * i + 2] = 0;
			palettes[p].pal[4 * i + 3] = 0;
		}
		p++;
	}
	fclose(f);

	palnum = 0;
	maxPalnum = p;
	_vm->_graphics->setPalette(0, 256, palettes[palnum].pal, RDPAL_INSTANT);
	lut = lut2 = lookup[0];
	curpal = -1;
	cr = 0;
	buildLookup(palnum, 256);
	lut2 = lookup[1];
	lutcalcnum = (BITDEPTH + palettes[palnum].end + 2) / (palettes[palnum].end + 2);
#else
	buildLookup2();
	overlay = (NewGuiColor*)calloc(640 * 400, sizeof(NewGuiColor));
	_vm->_system->show_overlay();
#endif

	// Open MPEG2 stream
	mpgfile = new File();
	sprintf(tempFile, "%s.mp2", name);
	if (!mpgfile->open(tempFile)) {
		warning("Cutscene: Could not open %s", tempFile);
		return false;
	}

	// Load and configure decoder
	decoder = mpeg2_init();
	if (decoder == NULL) {
		warning("Cutscene: Could not allocate an MPEG2 decoder");
		return false;
	}

	info = mpeg2_info(decoder);
	framenum = 0;
	ticks = _vm->_system->get_msecs();

	// Play audio

#ifdef USE_VORBIS
	// TODO: There is no reason that this only allows OGG, and not MP3, or
	// any other format the mixer might support one day... is there?
	sndfile = new File;
	sprintf(tempFile, "%s.ogg", name);
	if (sndfile->open(tempFile)) {
		bgSoundStream = makeVorbisStream(sndfile, sndfile->size());
		_vm->_mixer->playInputStream(&bgSound, bgSoundStream, false, 255, 0, -1);
	}

#endif

	return true;
#else /* USE_MPEG2 */
	return false;
#endif
}


#ifdef BACKEND_8BIT
/**
 * Build 'Best-Match' RGB lookup table
 */
void AnimationState::buildLookup(int p, int lines) {
	int y, cb;
	int r, g, b, ii;

	if (p >= maxPalnum)
		return;
  
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

bool AnimationState::checkPaletteSwitch() {
	// if we have reached the last image with this palette, switch to new one
	if (framenum == palettes[palnum].end) {
		unsigned char *l = lut2;
		palnum++;
		_vm->_graphics->setPalette(0, 256, palettes[palnum].pal, RDPAL_INSTANT);
		lutcalcnum = (BITDEPTH + palettes[palnum].end - (framenum + 1) + 2) / (palettes[palnum].end - (framenum + 1) + 2);
		lut2 = lut;
		lut = l;
		return true;
	}

	return false;
}


#else

bool AnimationState::lookupInit = false;
NewGuiColor AnimationState::lookup2[BITDEPTH * BITDEPTH * 256];

void AnimationState::buildLookup2() {
	if (lookupInit)
		return;

	lookupInit = true;

	int y, cb, cr;
	int r, g, b;
	int pos = 0;

	for (cr = 0; cr < BITDEPTH; cr++) {
		for (cb = 0; cb < BITDEPTH; cb++) {
			for (y = 0; y < 256; y++) {
				r = ((y - 16) * 256 + (int) (256 * 1.596) * ((cr << SHIFT) - 128)) / 256;
				g = ((y - 16) * 256 - (int) (0.813 * 256) * ((cr << SHIFT) - 128) - (int) (0.391 * 256) * ((cb << SHIFT) - 128)) / 256;
				b = ((y - 16) * 256 + (int) (2.018 * 256) * ((cb << SHIFT) - 128)) / 256;

				if (r < 0)
					r = 0;
				if (r > 255)
					r = 255;
				if (g < 0)
					g = 0;
				if (g > 255)
					g = 255;
				if (b < 0)
					b = 0;
				if (b > 255)
					b = 255;

				lookup2[pos++] = _vm->_system->RGBToColor(r, g, b);
			}
		}
	}
}


void AnimationState::plotYUV(NewGuiColor *lut, int width, int height, byte *const *dat) {

	NewGuiColor *ptr = overlay + (400 - height) / 2 * 640 + (640 - width) / 2;

	int x, y;

	int ypos = 0;
	int cpos = 0;
	int linepos = 0;

	for (y = 0; y < height; y += 2) {
		for (x = 0; x < width; x += 2) {
			int i = ((((dat[2][cpos] + ROUNDADD) >> SHIFT) * BITDEPTH) + ((dat[1][cpos] + ROUNDADD)>>SHIFT)) * 256;
			cpos++;

			ptr[linepos               ] = lut[i + dat[0][        ypos  ]];
			ptr[RENDERWIDE + linepos++] = lut[i + dat[0][width + ypos++]];
			ptr[linepos               ] = lut[i + dat[0][        ypos  ]];
			ptr[RENDERWIDE + linepos++] = lut[i + dat[0][width + ypos++]];

                }
		linepos += (2 * 640 - width);
		ypos += width;
	}
}

void AnimationState::drawTextObject(SpriteInfo *s, uint8 *src) {
	NewGuiColor *dst = overlay + RENDERWIDE * (s->y - 40) + s->x;

	// FIXME: These aren't the "right" colours, but look good to me.

	NewGuiColor pen = _vm->_system->RGBToColor(255, 255, 255);
	NewGuiColor border = _vm->_system->RGBToColor(0, 0, 0);

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

void AnimationState::updateDisplay(void) {
	_vm->_system->copy_rect_overlay(overlay, 640, 0, 40, 640, 400);
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
				 * we calculate the actual frame by taking the delivered audio samples
				 * we add 2 frames as the number of samples delivered is higher than the
				 * number actually played due to buffering
				 *
				 * we then try to stay inside +- 1 frame of this calculated frame number by
				 * dropping frames if we run behind and delaying if we are too fast
				 */

#ifdef BACKEND_8BIT
				if (checkPaletteSwitch() || (bgSoundStream == NULL) ||
					(bgSoundStream->getSamplesPlayed() * 12 / bgSoundStream->getRate()) < (framenum + 3)){

					_vm->_graphics->plotYUV(lut, sequence_i->width, sequence_i->height, info->display_fbuf->buf);

					if (bgSoundStream) {
						while ((bgSoundStream->getSamplesPlayed() * 12 / bgSoundStream->getRate()) < framenum + 1)
							_vm->_system->delay_msecs(10);
					} else {
						ticks += 83;
						_vm->sleepUntil(ticks);
					}

					_vm->_graphics->setNeedFullRedraw();

				} else
					warning("dropped frame %i", framenum);

				buildLookup(palnum + 1, lutcalcnum);

#else

				if ((bgSoundStream == NULL) ||
					(bgSoundStream->getSamplesPlayed()*12/bgSoundStream->getRate()) < (framenum+3)){

					plotYUV(lookup2, sequence_i->width, sequence_i->height, info->display_fbuf->buf);

					if (bgSoundStream) {
						while ((bgSoundStream->getSamplesPlayed()*12/bgSoundStream->getRate()) < framenum + 1)
							_vm->_system->delay_msecs(10);
					} else {
						ticks += 83;
						_vm->sleepUntil(ticks);
					}

				} else
					warning("dropped frame %i", framenum);

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
	int frameCounter = 0, textCounter = 0;
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
				_vm->_mixer->playRaw(&handle, text[textCounter]->speech, text[textCounter]->speechBufferSize, 22050, flags);
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

#ifdef BACKEND_8BIT
		_vm->_graphics->updateDisplay(true);
#else
		anim->updateDisplay();
		_vm->_graphics->updateDisplay(false);
#endif

		KeyboardEvent ke;

		if ((_vm->_input->readKey(&ke) == RD_OK && ke.keycode == 27) || _vm->_quit) {
			_vm->_mixer->stopHandle(handle);
			skipCutscene = true;
			break;
		}

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

	if (!skipCutscene)
		_vm->_sound->playLeadOut(musicOut);

	_vm->_graphics->setPalette(0, 256, oldPal, RDPAL_INSTANT);

	delete anim;

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
