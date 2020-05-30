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

#ifndef KYRA_TEXT_EOB_SEGACD_H
#define KYRA_TEXT_EOB_SEGACD_H

#include "kyra/text/text_rpg.h"

namespace Kyra {

class EoBEngine;
class Screen_EoB;

class TextDisplayer_SegaCD : public TextDisplayer_rpg {
public:
	TextDisplayer_SegaCD(EoBEngine *engine, Screen_EoB *scr);
	virtual ~TextDisplayer_SegaCD();

	void printDialogueText(int id, const char *string1, const char *string2) override;
	void printDialogueText(const char *str, bool wait = false) override;
	void printShadedText(const char *str, int x = -1, int y = -1, int textColor = -1, int shadowColor = -1, int pitchW = -1, int pitchH = -1, int marginRight = 0, bool screenUpdate = true) override;
	int clearDim(int dim) override;

private:
	void displayText(char *str, ...) override;
	uint8 fetchCharacter(char *dest, const char *&src);
	void linefeed();

	void clearTextBufferLine(uint16 y, uint16 lineHeight, uint16 pitch, uint8 col);
	void copyTextBufferLine(uint16 srcY, uint16 dstY, uint16 lineHeight, uint16 pitch);

	Screen_EoB *_screen;
	SegaRenderer *_renderer;
	EoBEngine *_engine;
	uint8 *_msgRenderBuffer;
	uint32 _msgRenderBufferSize;

	int _curDim;
	int _curPosY;
	int _curPosX;
	int _textColor;

	static const ScreenDim _dimTable[6];
};

} // End of namespace Kyra

#endif

#endif // ENABLE_EOB
