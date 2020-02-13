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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ILLUSIONS_BBDOU_BBDOU_CREDITS_H
#define ILLUSIONS_BBDOU_BBDOU_CREDITS_H

#include "illusions/specialcode.h"
#include "illusions/thread.h"

namespace Illusions {

class IllusionsEngine_BBDOU;
class BbdouSpecialCode;
class Control;

struct CreditsItem {
	bool isUsed;
	uint32 objectId;
};

const uint kCreditsItemsCount = 64;

class BbdouCredits {
public:
	BbdouCredits(IllusionsEngine_BBDOU *vm);
	~BbdouCredits();
	void start(uint32 endSignalPropertyId, float speedModifier);
	void stop();
	void drawNextLine();
	void updateTexts(int yIncr);
protected:
	IllusionsEngine_BBDOU *_vm;
	uint32 _endSignalPropertyId;
	uint32 _currFontId;
	uint _currLineIndex;
	bool _split;
	CreditsItem _items[kCreditsItemsCount];
	const char *getText(uint index);
	void drawTextToControl(uint32 objectId, const char *text, uint alignment);
	bool readNextLine(uint &leftIndex, uint &rightIndex);
	void initCreditsItems();
	void freeCreditsItems();
	uint32 getNextFreeObjectId();
	void removeText(uint32 objectId);
	void resetObjectPos(uint32 objectId);
	void createCreditsThread(float speedModifier);
};

class CreditsThread : public Thread {
public:
	CreditsThread(IllusionsEngine_BBDOU *vm, BbdouCredits *credits, uint32 threadId, float speedModifier);
	int onUpdate() override;
	void onNotify() override;
	void onResume() override;
	void onTerminated() override;
public:
	BbdouCredits *_credits;
	float _speedModifier;
	float _lastFraction;
	uint32 _lastUpdateTime;
};

} // End of namespace Illusions

#endif // ILLUSIONS_BBDOU_BBDOU_CREDITS_H
