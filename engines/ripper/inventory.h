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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RIPPER_INVENTORY_H
#define RIPPER_INVENTORY_H

#include "common/array.h"
#include "common/str.h"

namespace Ripper {

class DialogueChooser;
class ResourceManager;
class RipperEngine;

class Inventory {
public:
	enum Result {
		kCancelled,
		kUsed,
		kLoadFailed
	};

	explicit Inventory(RipperEngine *engine);
	~Inventory();

	bool initialize(ResourceManager &resources);
	bool grant(uint unlockFlag, const char *source);
	Result run(const Common::String &sceneLabel, int initialUnlockFlag = -1,
		uint *usedUnlockFlag = nullptr);
	Result grantAndRun(uint unlockFlag, const Common::String &sceneLabel,
		const char *source);
	bool isAvailable(uint unlockFlag) const;

private:
	enum ChoiceResult {
		kChoiceRejected,
		kChoiceUsed,
		kChoiceFailed
	};

	ChoiceResult executeChoice(uint unlockFlag, const Common::String &sceneLabel);
	bool rebuildChooser(int initialUnlockFlag);
	void closeChooser(const char *reason);

	RipperEngine *_engine;
	DialogueChooser *_chooser;
	Common::Array<Common::String> _gameText;
};

} // End of namespace Ripper

#endif // RIPPER_INVENTORY_H
