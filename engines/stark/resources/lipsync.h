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

#ifndef STARK_RESOURCES_LIPSYNC_H
#define STARK_RESOURCES_LIPSYNC_H

#include "common/str.h"

#include "engines/stark/resources/object.h"

namespace Stark {

class VisualActor;

namespace Formats {
class XRCReadStream;
}

namespace Resources {

class ItemVisual;
class ModelItem;
class TextureSet;

/**
 * Speech lipsync data
 */
class LipSync : public Object {
public:
	static const Type::ResourceType TYPE = Type::kLipSync;

	LipSync(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~LipSync();

	// Resource API
	void readData(Formats::XRCReadStream *stream) override;
	void onGameLoop() override;
	void saveLoadCurrent(ResourceSerializer *serializer) override;

	/** Set the item for which the facial texture should be updated according to the lipsync data */
	void setItem(ItemVisual *item, bool playTalkAnim);

	/** Removes all item related data from the LipSync object */
	void reset();

protected:
	void printData() override;

	Common::Array<char> _shapes;
	ItemVisual *_item;
	ModelItem *_sceneItem;
	TextureSet *_faceTexture;
	VisualActor *_visual;

	bool _checkForNewVisual;
	bool _enabled;
	uint32 _positionMs;
};

} // End of namespace Resources
} // End of namespace Stark

#endif // STARK_RESOURCES_LIPSYNC_H
