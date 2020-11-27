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

#ifdef ENABLE_EOB

#include "kyra/engine/eob.h"
#include "kyra/graphics/screen_eob.h"
#include "kyra/graphics/screen_eob_segacd.h"
#include "kyra/text/text_eob_segacd.h"

namespace Kyra {

TextDisplayer_SegaCD::TextDisplayer_SegaCD(EoBEngine *engine, Screen_EoB *scr) : TextDisplayer_rpg(engine, scr), _engine(engine), _screen(scr), _renderer(scr->sega_getRenderer()),
	_curDim(0), _textColor(0xFF), _curPosY(0), _curPosX(0) {
	assert(_renderer);
	_msgRenderBufferSize = 320 * 48;
	_msgRenderBuffer = new uint8[_msgRenderBufferSize];
	memset(_msgRenderBuffer, 0, _msgRenderBufferSize);
}

TextDisplayer_SegaCD::~TextDisplayer_SegaCD() {
	delete[] _msgRenderBuffer;
}

void TextDisplayer_SegaCD::printDialogueText(int id, const char *string1, const char *string2) {
	if (string1 && string2) {
		_engine->runDialogue(id, 2, 2, string1, string2);
	} else {
		_screen->hideMouse();
		_engine->seq_segaPlaySequence(id);
		_screen->showMouse();
	}
}

void TextDisplayer_SegaCD::printDialogueText(const char *str, bool wait) {
	int cs = _screen->setFontStyles(Screen::FID_8_FNT, _vm->gameFlags().lang == Common::JA_JPN ? Font::kStyleNone : Font::kStyleFullWidth);
	clearDim(_curDim);

	if (wait) {
		printShadedText(str, 32, 12);
		_engine->resetSkipFlag();
		_renderer->render(0);
		_screen->updateScreen();
		_engine->delay(500);
	} else {
		printShadedText(str, 0, 0);
		_renderer->render(0);
		_screen->updateScreen();
	}

	_screen->setFontStyles(Screen::FID_8_FNT, cs);
}

void TextDisplayer_SegaCD::printShadedText(const char *str, int x, int y, int textColor, int shadowColor, int pitchW, int pitchH, int marginRight, bool screenUpdate) {
	const ScreenDim *s = &_dimTable[_curDim];
	if (x == -1)
		x = s->sx;
	if (y == -1)
		y = s->sy;
	if (textColor == -1)
		textColor = s->unk8;
	if (shadowColor == -1)
		shadowColor = 0;
	if (pitchW == -1)
		pitchW = s->w;
	if (pitchH == -1)
		pitchH = s->h;

	_screen->setTextMarginRight(pitchW - marginRight);
	_screen->printShadedText(str, x, y, textColor, 0, shadowColor, pitchW >> 3);

	if (!screenUpdate)
		return;

	if (s->unkE) {
		for (int i = 0; i < (pitchH >> 3); ++i)
			_screen->sega_loadTextBufferToVRAM(i * (pitchW << 2), ((s->unkC & 0x7FF) + i * s->unkE) << 5, pitchW << 2);
	} else {
		_screen->sega_loadTextBufferToVRAM(0, (s->unkC & 0x7FF) << 5, (pitchW * pitchH) >> 1);
	}
}

int TextDisplayer_SegaCD::clearDim(int dim) {
	int res = _curDim;
	_curDim = dim;
	_curPosY = _curPosX = 0;
	const ScreenDim *s = &_dimTable[dim];
	_renderer->memsetVRAM((s->unkC & 0x7FF) << 5, s->unkA, (s->w * s->h) >> 1);
	_screen->sega_clearTextBuffer(s->unkA);
	memset(_msgRenderBuffer, 0, _msgRenderBufferSize);
	return res;
}

void TextDisplayer_SegaCD::displayText(char *str, ...) {
	_screen->sega_setTextBuffer(_msgRenderBuffer, _msgRenderBufferSize);
	int cs = _screen->setFontStyles(Screen::FID_8_FNT, Font::kStyleFullWidth);
	_screen->setFontStyles(Screen::FID_8_FNT, cs | Font::kStyleFullWidth);
	char tmp[3] = "  ";
	int posX = _curPosX;
	bool updated = false;

	va_list args;
	va_start(args, str);
	int tc = va_arg(args, int);
	va_end(args);

	if (tc != -1)
		SWAP(_textColor, tc);

	for (const char *pos = str; *pos; updated = false) {
		uint8 cmd = fetchCharacter(tmp, pos);

		if (_dimTable[_curDim].h < _curPosY + _screen->getFontHeight()) {
			_curPosY -= _screen->getFontHeight();
			linefeed();
		}

		if (cmd == 6) {
			_textColor = (uint8)*pos++;
		} else if (cmd == 2) {
			pos++;
		} else if (cmd == 13) {
			_curPosX = 0;
			_curPosY += _screen->getFontHeight();
		} else if (cmd == 9) {
			_curPosX = posX;
			_curPosY += _screen->getFontHeight();
		} else {
			if (((tmp[0] == ' ' || (tmp[0] == '\x81' && tmp[1] == '\x40')) && (_curPosX + _screen->getTextWidth(tmp) + _screen->getTextWidth((const char*)(pos), true) >= _dimTable[_curDim].w)) || (_curPosX + _screen->getTextWidth(tmp) >= _dimTable[_curDim].w)) {
				// Skip space at the beginning of the new line
				if (tmp[0] == ' ' || (tmp[0] == '\x81' && tmp[1] == '\x40'))
					fetchCharacter(tmp, pos);

				_curPosX = 0;
				_curPosY += _screen->getFontHeight();
				if (_dimTable[_curDim].h < _curPosY + _screen->getFontHeight()) {
					_curPosY -= _screen->getFontHeight();
					linefeed();
				}
			}

			printShadedText(tmp, _curPosX, _curPosY, _textColor);
			_curPosX += _screen->getTextWidth(tmp);
			updated = true;
		}
	}

	if (!updated)
		printShadedText("", _curPosX, _curPosY, _textColor);

	if (tc != -1)
		SWAP(_textColor, tc);

	_renderer->render(Screen_EoB::kSegaRenderPage);
	_screen->setFontStyles(Screen::FID_8_FNT, cs);
	_screen->copyRegion(8, 176, 8, 176, 280, 24, Screen_EoB::kSegaRenderPage, 0, Screen::CR_NO_P_CHECK);
	_screen->sega_setTextBuffer(0, 0);
}

uint8 TextDisplayer_SegaCD::fetchCharacter(char *dest, const char *&src) {
	uint8 c = (uint8)*src++;

	if (c <= (uint8)'\r') {
		dest[0] = '\0';
		return c;
	}

	dest[0] = (char)c;
	dest[1] = (c <= 0x7F || (c >= 0xA1 && c <= 0xDF)) ? '\0' : *src++;

	return 0;
}

void TextDisplayer_SegaCD::linefeed() {
	copyTextBufferLine(_screen->getFontHeight(), 0, (_dimTable[_curDim].h & ~7) - _screen->getFontHeight(), _dimTable[_curDim].w >> 3);
	clearTextBufferLine(_screen->getFontHeight(), _screen->getFontHeight(), _dimTable[_curDim].w >> 3, _dimTable[_curDim].unkA);
}

void TextDisplayer_SegaCD::clearTextBufferLine(uint16 y, uint16 lineHeight, uint16 pitch, uint8 col) {
	uint32 *dst = (uint32*)(_msgRenderBuffer + (((y >> 3) * pitch) << 5) + ((y & 7) << 2));
	int ln = y;
	uint32 c = col | (col << 8) | (col << 16) | (col << 24);
	while (lineHeight--) {
		uint32 *dst2 = dst;
		for (uint16 w = pitch; w; --w) {
			*dst = c;
			dst += 8;
		}
		dst = dst2 + 1;
		if (((++ln) & 7) == 0)
			dst += ((pitch - 1) << 3);
	}
}


void TextDisplayer_SegaCD::copyTextBufferLine(uint16 srcY, uint16 dstY, uint16 lineHeight, uint16 pitch) {
	uint32 *src = (uint32*)(_msgRenderBuffer + (((srcY >> 3) * pitch) << 5) + ((srcY & 7) << 2));
	uint32 *dst = (uint32*)(_msgRenderBuffer + (((dstY >> 3) * pitch) << 5) + ((dstY & 7) << 2));
	int src_ln = srcY;
	int dst_ln = dstY;

	while (lineHeight--) {
		uint32 *src2 = src;
		uint32 *dst2 = dst;

		for (uint16 w = pitch; w; --w) {
			*dst = *src;
			src += 8;
			dst += 8;
		}

		src = src2 + 1;
		dst = dst2 + 1;

		if (((++dst_ln) & 7) == 0)
			dst += ((pitch - 1) << 3);
		if (((++src_ln) & 7) == 0)
			src += ((pitch - 1) << 3);
	}
}

const ScreenDim TextDisplayer_SegaCD::_dimTable[6] = {
	{ 0x0001, 0x0017, 0x0118, 0x0018, 0xff, 0x44, 0x2597, 0x0000 },
	{ 0x0012, 0x0009, 0x00a0, 0x0080, 0xff, 0x99, 0x0153, 0x0028 },
	{ 0x0001, 0x0014, 0x0130, 0x0030, 0xff, 0xee, 0xe51c, 0x0000 },
	{ 0x0001, 0x0017, 0x00D0, 0x0030, 0xff, 0x00, 0x0461, 0x0000 },
	{ 0x0000, 0x0000, 0x00F0, 0x0100, 0xff, 0x00, 0x600A, 0x0000 },
	{ 0x0000, 0x0000, 0x0140, 0x00B0, 0xff, 0x11, 0x4001, 0x0000 }
};

} // End of namespace Kyra

#endif // ENABLE_EOB
