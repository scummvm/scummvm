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

#ifndef HOPKINS_ANIM_H
#define HOPKINS_ANIM_H

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/path.h"
#include "graphics/surface.h"

namespace Hopkins {

struct BankItem {
	byte *_data;
	bool _loadedFl;
	Common::Path _filename;
	int _fileHeader;
	int _objDataIdx;
};

struct BqeAnimItem {
	byte *_data;
	bool _enabledFl;
};

class HopkinsEngine;

class AnimationManager {
private:
	bool _clearAnimationFl;

	HopkinsEngine *_vm;

	void initAnimBqe();
	int loadSpriteBank(int idx, const Common::Path &filename);
	void searchAnim(const byte *data, int animIndex, int count);

public:
	BqeAnimItem _animBqe[35];
	BankItem Bank[8];

	AnimationManager(HopkinsEngine *vm);
	void clearAll();

	void loadAnim(const Common::Path &animName);
	void clearAnim();
	void playAnim(const Common::Path &hiresName, const Common::Path &lowresName, uint32 rate1, uint32 rate2, uint32 rate3, bool skipSeqFl = false);
	void playAnim2(const Common::Path &hiresName, const Common::Path &lowresName, uint32 rate1, uint32 rate2, uint32 rate3);
	void playSequence(const Common::Path &file, uint32 rate1, uint32 rate2, uint32 rate3, bool skipEscFl, bool skipSeqFl, bool noColFl = false);
	void playSequence2(const Common::Path &file, uint32 rate1, uint32 rate2, uint32 rate3, bool skipSeqFl = false);

	void setClearAnimFlag()   { _clearAnimationFl = true; }
	void unsetClearAnimFlag() { _clearAnimationFl = false; }
};

} // End of namespace Hopkins

#endif /* HOPKINS_ANIM_H */
