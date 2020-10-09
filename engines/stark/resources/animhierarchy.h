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

#ifndef STARK_RESOURCES_ANIM_HIERARCHY_H
#define STARK_RESOURCES_ANIM_HIERARCHY_H

#include "common/str.h"

#include "engines/stark/resources/object.h"
#include "engines/stark/resourcereference.h"

namespace Stark {

class Visual;

namespace Formats {
class XRCReadStream;
}

namespace Resources {

class Anim;
class BonesMesh;
class ItemVisual;
class TextureSet;

/**
 * An animation hierarchy is a container resource referencing the available
 * animations for an item.
 *
 * This resource keeps track of the currently selected animation.
 */
class AnimHierarchy : public Object {
public:
	static const Type::ResourceType TYPE = Type::kAnimHierarchy;

	AnimHierarchy(Object *parent, byte subType, uint16 index, const Common::String &name);
	~AnimHierarchy() override;

	// Resource API
	void readData(Formats::XRCReadStream *stream) override;
	void onAllLoaded() override;

	/** Set and apply the current animation kind for an item */
	void setItemAnim(ItemVisual *item, int32 activity);

	/** Unselect the current animation and remove it from an item */
	void unselectItemAnim(ItemVisual *item);

	/** Apply the current animation to an item */
	void selectItemAnim(ItemVisual *item);

	/** Obtain the currently selected animation */
	Anim *getCurrentAnim();

	/** Retrieve the first bone mesh from the anim hierarchy children, if any */
	BonesMesh *findBonesMesh();

	/**
	 * Retrieve the first texture of the appropriate type from the anim
	 * hierarchy children, if any
	 */
	TextureSet *findTextureSet(uint32 textureType);

	Visual *getVisualForUsage(uint32 usage);

	/** Randomize an idle action animation */
	Anim *getIdleActionAnim() const;

protected:
	void loadActivityAnimations();
	void loadIdleAnimations();

	Anim *getAnimForActivity(uint32 activity);
	void printData() override;

	Common::Array<ResourceReference> _animationReferences;
	Common::Array<Anim *> _activityAnimations;
	Common::Array<Anim *> _idleAnimations;

	ResourceReference _parentAnimHierarchyReference;

	float _field_5C;
	uint32 _currentActivity;
	Anim *_currentAnim;
	uint32 _idleActionsFrequencySum;
};

} // End of namespace Resources
} // End of namespace Stark

#endif // STARK_RESOURCES_ANIM_HIERARCHY_H
