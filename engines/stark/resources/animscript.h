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

#include "engines/stark/resources/object.h"

namespace Stark {

namespace Formats {
class XRCReadStream;
}

namespace Resources {

class Anim;
class AnimScriptItem;

/**
 * Animation scripts control the currently displayed frame for images animation
 * resources.
 *
 * Animation scripts contain animation script items defining which frames
 * should be displayed and when.
 *
 * Animation scripts also allow to play sounds.
 */
class AnimScript : public Object {
public:
	static const Type::ResourceType TYPE = Type::kAnimScript;

	AnimScript(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~AnimScript();

	// Resource API
	void onAllLoaded() override;
	void onGameLoop() override;
	void saveLoad(ResourceSerializer *serializer) override;

	/** Go to a script item. Cancel any delay so that it is shown immediately. */
	void goToScriptItem(AnimScriptItem *item);

	/** Is the current script item later in the script when compared to the specified one? */
	bool hasReached(AnimScriptItem *item);

	/** Has the script completed playing the last script item at least once since started? */
	bool isDone() const;

protected:
	void goToNextItem();
	int32 findItemIndex(AnimScriptItem *item);

	Anim *_anim;
	Common::Array<AnimScriptItem *> _items;

	int32 _nextItemIndex;
	int32 _msecsToNextUpdate;
	bool _done;
};

/**
 * Animation script element
 *
 * Has a type defining the operation to perform,
 * an argument and a duration.
 */
class AnimScriptItem : public Object {
public:
	static const Type::ResourceType TYPE = Type::kAnimScriptItem;

	enum Opcodes {
		kDisplayFrame        = 0,
		kPlayAnimSound       = 1,
		kGoToItem            = 2,
		kDisplayRandomFrame  = 3,
		kSleepRandomDuration = 4,
		kPlayStockSound      = 5
	};

	AnimScriptItem(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~AnimScriptItem();

	// Resource API
	void readData(Formats::XRCReadStream *stream) override;

	/** Obtain the operation code */
	uint32 getOpcode() const { return _opcode; }

	/** Obtain the operation parameter */
	uint32 getOperand() const { return _operand; }

	/** Obtain the operation duration */
	uint32 getDuration() const { return _duration; }

protected:
	void printData() override;

	uint32 _opcode;
	uint32 _operand;
	uint32 _duration;
};

} // End of namespace Resources
} // End of namespace Stark

#endif // STARK_RESOURCES_ANIM_SCRIPT_H
