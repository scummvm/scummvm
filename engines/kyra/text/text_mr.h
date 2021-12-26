/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef KYRA_TEXT_MR_H
#define KYRA_TEXT_MR_H

#include "kyra/text/text.h"

#include "kyra/engine/kyra_mr.h"

namespace Kyra {

class TextDisplayer_MR : public TextDisplayer {
friend class KyraEngine_MR;
public:
	TextDisplayer_MR(KyraEngine_MR *vm, Screen_MR *screen);

	char *preprocessString(const char *str) override;
	int dropCRIntoString(char *str, int minOffs, int maxOffs);

	void printText(const Common::String &str, int x, int y, uint8 c0, uint8 c1, uint8 c2) override;

	void restoreScreen();

	void calcWidestLineBounds(int &x1, int &x2, int w, int x) override;
protected:
	KyraEngine_MR *_vm;
	Screen_MR *_screen;
};

} // End of namespace Kyra

#endif
