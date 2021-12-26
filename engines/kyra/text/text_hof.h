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

#ifndef KYRA_TEXT_HOF_H
#define KYRA_TEXT_HOF_H

#include "kyra/text/text.h"

namespace Kyra {

class Screen_v2;
class KyraEngine_HoF;

class TextDisplayer_HoF : public TextDisplayer {
friend class KyraEngine_HoF;
public:
	TextDisplayer_HoF(KyraEngine_HoF *vm, Screen_v2 *screen);

	void backupTalkTextMessageBkgd(int srcPage, int dstPage);
	void restoreTalkTextMessageBkgd(int srcPage, int dstPage) override;
	void restoreScreen();

	void printCustomCharacterText(const char *src, int x, int y, uint8 c1, int srcPage, int dstPage);

	char *preprocessString(const char *str) override;
	void calcWidestLineBounds(int &x1, int &x2, int w, int x) override;
private:
	KyraEngine_HoF *_vm;
};

} // End of namespace Kyra

#endif
