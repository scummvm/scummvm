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

#if defined(ENABLE_EOB) || defined(ENABLE_LOL)

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

	/*void setupField(int dim, bool mode);

	void printDialogueText(int stringId, const char *pageBreakString);
	void printDialogueText(const char *str, bool wait = false);
	void printMessage(const char *str, int textColor = -1, ...);*/
	void printMessageAtPos(const char *str, int x = -1, int y = -1, int textColor = -1, int shadowColor = -1) override;

	int clearDim(int dim) override;
	//void clearCurDim() override;
	
	/*void resetDimTextPositions(int dim);
	void resetPageBreakString();
	void setPageBreakFlag();
	void removePageBreakFlag();
	*/
	//void allowPageBreak(bool mode) { _allowPageBreak = mode; }
	//void setWaitButtonMode(int mode) { _waitButtonMode = mode; }
	//int lineCount() const { return _lineCount; }

private:
	Screen_EoB *_screen;
	SegaRenderer *_renderer;
	int _curDim;

	static const ScreenDim _dimTable[4];
};

} // End of namespace Kyra

#endif

#endif // ENABLE_EOB || ENABLE_LOL
