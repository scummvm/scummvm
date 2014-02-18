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

#ifndef HOPKINS_TALK_H
#define HOPKINS_TALK_H

#include "common/scummsys.h"
#include "common/str.h"

namespace Hopkins {

class HopkinsEngine;

#define MIN_LETTERS_PER_LINE 65

class TalkManager {
private:
	HopkinsEngine *_vm;

	Common::String _questionsFilename;
	Common::String _answersFilename;
	byte *_characterBuffer;
	byte *_characterPalette;
	size_t _characterSize;
	int _dialogueMesgId1, _dialogueMesgId2;
	int _dialogueMesgId3, _dialogueMesgId4;
	int _paletteBufferIdx;

	void getStringFromBuffer(int srcStart, Common::String &dest, const char *srcData);
	int dialogQuestion(bool animatedFl);
	int dialogAnswer(int idx, bool animatedFl);
	void searchCharacterPalette(int startIdx, bool dark);
	void dialogWait();
	void dialogTalk();
	void dialogEndTalk();
	void startCharacterAnim0(int startIndedx, bool readOnlyFl);
	void initCharacterAnim();
	void clearCharacterAnim();
	bool searchCharacterAnim(int idx, const byte *bufPerso, int animId, int bufferSize);
	int countBoxLines(int idx, const Common::String &file);
	void dialogAnim();
	void displayBobDialogAnim(int idx);

public:
	byte *_characterAnim;
	byte *_characterSprite;

	TalkManager(HopkinsEngine *vm);

	void startStaticCharacterDialogue(const Common::String &filename);
	void startAnimatedCharacterDialogue(const Common::String &filename);
	void animateObject(const Common::String &filename);
	void handleAnswer(int zone, int verb);
	void handleForestAnswser(int zone, int verb);
};

} // End of namespace Hopkins

#endif /* HOPKINS_TALK_H */
