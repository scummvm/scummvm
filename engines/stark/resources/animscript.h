/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef STARK_RESOURCES_ANIM_SCRIPT_H
#define STARK_RESOURCES_ANIM_SCRIPT_H

#include "common/str.h"

#include "engines/stark/resources/resource.h"

namespace Stark {

class Anim;
class AnimScriptItem;
class XRCReadStream;

class AnimScript : public Resource {
public:
	static const ResourceType::Type TYPE = ResourceType::kAnimScript;

	AnimScript(Resource *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~AnimScript();

	// Resource API
	void onAllLoaded() override;
	void onGameLoop(uint msecs) override;

protected:
	void goToNextItem();

	Anim *_anim;
	Common::Array<AnimScriptItem *> _items;

	int32 _nextItemIndex;
	int32 _msecsToNextUpdate;
};

class AnimScriptItem : public Resource {
public:
	static const ResourceType::Type TYPE = ResourceType::kAnimScriptItem;

	enum Opcodes {
		kDisplayFrame = 0,
		kPlayAnimSound = 1,
		kGoToItem = 2,
		kDisplayRandomFrame = 3,
		kSleepRandomDuration = 4,
		kPlayStockSound = 5
	};

	AnimScriptItem(Resource *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~AnimScriptItem();

	// Resource API
	void readData(XRCReadStream *stream) override;

	uint32 getOpcode() const { return _opcode; }
	uint32 getOperand() const { return _operand; }
	uint32 getDuration() const { return _duration; }

protected:
	void printData() override;

	uint32 _opcode;
	uint32 _operand;
	uint32 _duration;
};

} // End of namespace Stark

#endif // STARK_RESOURCES_ANIM_SCRIPT_H
